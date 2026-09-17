"""Select an OpenMV CI storage profile and verify its downloadable firmware."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import zlib

sys.dont_write_bytecode = True
DEFAULT_PROJECT = Path(__file__).resolve().parents[2] / "project/Titan_Mini_Openmv"
HEADER = struct.Struct("<8sIIII32s8s")
VISION_START = 0x68C00000
VISION_SIZE = 4 * 1024 * 1024
FIRMWARE_FILES = ("rtthread.hex", "openmv-vision.bin")


def configure(project, storage):
    import kconfiglib

    sdk = project.parents[1]
    os.environ.update(BSP_ROOT=project.as_posix(), BSP_DIR=project.as_posix(),
                      RTT_ROOT=(sdk / "rt-thread").as_posix(), RTT_DIR=(sdk / "rt-thread").as_posix(),
                      SDK_LIB_DIR=(sdk / "libraries").as_posix(),
                      TITAN_OPENMV_SDK_LIB_ROOT=(sdk / "libraries").as_posix())
    os.chdir(project)
    config = kconfiglib.Kconfig("Kconfig")
    config.load_config(".config")
    selected = "BSP_OPENMV_STORAGE_" + storage.upper()
    config.syms[selected].set_value(2)
    if config.syms[selected].str_value != "y":
        raise ValueError("Cannot select storage profile: " + storage)
    if storage == "flash" and any(config.syms[name].str_value == "y"
                                   for name in ("RT_USING_SDIO", "BSP_USING_SDHI", "BSP_USING_SDHI0")):
        raise ValueError("Flash profile still enables an SD host")
    if storage == "sd" and config.syms["BSP_USING_SDHI0"].str_value != "y":
        raise ValueError("SD profile does not enable SDHI0")
    config.write_config(".config")
    sys.path.insert(0, str(sdk / "rt-thread/tools"))
    from menuconfig import mk_rtconfig
    mk_rtconfig(".config")
    header = (project / "rtconfig.h").read_text()
    if '#include "rtconfig_project.h"' not in header or "#define " + selected not in header:
        raise ValueError("Generated configuration lost the OpenMV profile")
    print("OpenMV storage profile: " + storage)


def tool(name):
    executable = "arm-none-eabi-" + name + (".exe" if os.name == "nt" else "")
    directory = os.environ.get("RTT_EXEC_PATH")
    path = str(Path(directory) / executable) if directory else shutil.which(executable)
    if not path or not Path(path).is_file():
        raise ValueError("Missing ARM tool: " + executable)
    return path


def symbol(symbols, name):
    matches = [line.split() for line in symbols.splitlines() if line.split()[-1:] == [name]]
    if len(matches) != 1:
        raise ValueError("Missing or ambiguous ELF symbol: " + name)
    fields = matches[0]
    return int(fields[0], 16), int(fields[1], 16) if len(fields) == 4 else None


def package(project, storage):
    elf = project / "build/firmware/rtthread.elf"
    published = project / "firmware"
    ci_root = project / "build/ci"
    verification = ci_root / "verify"
    output = ci_root / "firmware"
    verification.mkdir(parents=True, exist_ok=True)
    output.mkdir(parents=True, exist_ok=True)
    expected = set(FIRMWARE_FILES) | {"manifest.json", "SHA256SUMS", "README.txt"}
    if any(path.name not in expected or not path.is_file() for path in output.iterdir()):
        raise ValueError("Unexpected files in the CI artifact directory")
    for name in FIRMWARE_FILES:
        if not (published / name).is_file() or not (published / name).stat().st_size:
            raise ValueError("Missing deployment file: " + name)

    def objcopy(*arguments):
        subprocess.run([tool("objcopy"), *map(str, arguments)], check=True)

    objcopy("-j", ".openmv_vision", "-O", "binary", elf, verification / "vision.bin")
    objcopy("-j", "__flash_readonly$$", "-O", "binary", elf, verification / "readonly.bin")
    objcopy("-R", ".openmv_vision", "-O", "ihex", elf, verification / "rtthread.hex")
    if (verification / "rtthread.hex").read_bytes() != (published / "rtthread.hex").read_bytes():
        raise ValueError("HEX differs from the linked ELF")

    payload = (verification / "vision.bin").read_bytes()
    blob = (published / "openmv-vision.bin").read_bytes()
    if len(blob) < HEADER.size:
        raise ValueError("Truncated vision image")
    magic, version, address, size, crc, digest, reserved = HEADER.unpack_from(blob)
    if magic != b"OMVVISN1" or address != VISION_START or not 0 < size <= VISION_SIZE:
        raise ValueError("Invalid vision image header")
    stored = blob[HEADER.size:]
    if version == 2:
        codec, stored_size = struct.unpack("<II", reserved)
        if codec != 1 or stored_size != len(stored):
            raise ValueError("Invalid compressed vision image extent")
        decoder = zlib.decompressobj(-13)
        decoded = decoder.decompress(stored, size + 1)
        if not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
            raise ValueError("Invalid compressed vision stream")
    elif version == 1 and reserved == bytes(8):
        decoded = stored
    else:
        raise ValueError("Unsupported vision image format")
    if len(decoded) != size or decoded != payload or zlib.crc32(decoded) != crc or hashlib.sha256(decoded).digest() != digest:
        raise ValueError("Vision image differs from the linked ELF")

    symbols = subprocess.check_output([tool("nm"), "-S", "--defined-only", str(elf)], text=True)
    base, _ = symbol(symbols, "__flash_readonly$$Base")
    readonly = (verification / "readonly.bin").read_bytes()

    def read_symbol(name):
        location, length = symbol(symbols, name)
        offset = location - base
        if length is None or offset < 0 or offset + length > len(readonly):
            raise ValueError("Symbol is outside the flash section: " + name)
        return readonly[offset:offset + length]

    if read_symbol("titan_vision_build_id") != digest:
        raise ValueError("HEX/ELF and vision BIN are not a matching firmware pair")
    backend = "sd" if storage == "sd" else "spi-flash"
    if read_symbol("titan_storage_backend_name") != backend.encode() + b"\0":
        raise ValueError("Artifact storage label differs from the compiled firmware")

    files = {}
    for name in FIRMWARE_FILES:
        data = (published / name).read_bytes()
        (output / name).write_bytes(data)
        files[name] = {"bytes": len(data), "sha256": hashlib.sha256(data).hexdigest()}
    compiler = subprocess.check_output([tool("gcc"), "--version"], text=True).splitlines()[0]
    manifest = {"schema": 1, "board": "RA8P1 Titan Mini OpenMV", "storage_backend": backend,
                "commit": os.environ.get("GITHUB_SHA"), "run_id": os.environ.get("GITHUB_RUN_ID"),
                "compiler": compiler, "files": files,
                "vision": {"format_version": version, "address": hex(address), "payload_bytes": size,
                           "file_bytes": len(blob), "payload_sha256": digest.hex()},
                "verification": "ELF, HEX, vision contents, build binding and storage identity",
                "hardware_tested": False}
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    (output / "SHA256SUMS").write_text("".join(files[name]["sha256"] + "  " + name + "\n" for name in FIRMWARE_FILES), encoding="utf-8")
    (output / "README.txt").write_text(
        "Titan Mini OpenMV firmware\nStorage: " + backend + "\nCommit: " + (manifest["commit"] or "local build") +
        "\n\n1. Flash rtthread.hex to the MCU internal memory.\n"
        "2. Copy openmv-vision.bin to the selected storage volume root.\n"
        "3. Safely eject the volume and reset the board.\n\n"
        "Always deploy HEX and BIN from this same artifact together.\n"
        "The sd build requires an SD card with a FAT filesystem.\n"
        "The spi-flash build uses the onboard W25Q64 filesystem.\n"
        "Models and examples are provided separately in the source repository.\n", encoding="utf-8")
    print(json.dumps({"artifact_directory": str(output), "storage_backend": backend,
                      "vision_payload_bytes": size, "files": files}, indent=2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project", type=Path, default=DEFAULT_PROJECT)
    parser.add_argument("command", choices=("configure", "package"))
    parser.add_argument("--storage", choices=("sd", "flash"), required=True)
    args = parser.parse_args()
    project = args.project.resolve()
    if not (project / "SConstruct").is_file():
        raise ValueError("Expected the Titan_Mini_Openmv project directory")
    (configure if args.command == "configure" else package)(project, args.storage)


if __name__ == "__main__":
    main()
