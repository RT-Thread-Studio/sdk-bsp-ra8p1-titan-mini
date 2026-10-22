"""Run OpenMV's SCons build with Python 3, even from Studio's legacy Env.

This launcher must remain compatible with Python 2.7. It only selects an
existing interpreter; it does not install tools or change Studio settings.
"""
from __future__ import print_function

import glob
import json
import os
import subprocess
import sys


def python_commands(environment):
    override = environment.get('TITAN_OPENMV_PYTHON')
    if override:
        yield [override]
        return
    yield [sys.executable]
    if os.name == 'nt':
        for base, relative in (('SYSTEMROOT', 'py.exe'),
                               ('LOCALAPPDATA', 'Programs/Python/Launcher/py.exe')):
            if environment.get(base):
                yield [os.path.join(environment[base], relative), '-3']
        yield ['py', '-3']
        if environment.get('LOCALAPPDATA'):
            pattern = os.path.join(environment['LOCALAPPDATA'], 'Programs', 'Python',
                                   'Python*', 'python.exe')
            for executable in sorted(glob.glob(pattern), reverse=True):
                yield [executable]
    names = ('python3.exe', 'python.exe') if os.name == 'nt' else ('python3', 'python')
    for directory in environment.get('PATH', '').split(os.pathsep):
        directory = directory.strip('"')
        # Windows Store aliases can open an installer instead of an interpreter.
        if not directory or os.path.basename(directory).lower() == 'windowsapps':
            continue
        for name in names:
            path = os.path.join(directory, name)
            if os.path.isfile(path):
                yield [path]


def studio_directories(environment, project_dir=None):
    origins = [project_dir or os.path.dirname(os.path.abspath(__file__))]
    if environment.get('ENV_ROOT'):
        origins.append(environment['ENV_ROOT'])
    seen = set()
    for origin in origins:
        directory = os.path.abspath(origin)
        while directory not in seen:
            seen.add(directory)
            yield directory
            parent = os.path.dirname(directory)
            if parent == directory:
                break
            directory = parent


def configure_host_tools(environment, project_dir=None):
    """Expose an installed portable Windows GCC/make/sh to mpy-cross."""
    if os.name != 'nt':
        return
    explicit = environment.get('OPENMV_HOST_TOOLS')
    candidates = [explicit] if explicit else [
        os.path.join(directory, 'platform', 'openmv-host-tools', 'w64devkit', 'bin')
        for directory in studio_directories(environment, project_dir)]
    for candidate in candidates:
        directory = os.path.abspath(candidate.strip('"'))
        if all(os.path.isfile(os.path.join(directory, name))
               for name in ('gcc.exe', 'make.exe', 'sh.exe')):
            environment['PATH'] = directory + os.pathsep + environment.get('PATH', '')
            environment.setdefault('MPY_HOST_CC', os.path.join(directory, 'gcc.exe'))
            print('OpenMV host tools: ' + directory)
            return
    if explicit:
        raise RuntimeError('OPENMV_HOST_TOOLS must contain gcc.exe, make.exe and sh.exe')


def resolve_toolchain(environment, project_dir=None):
    """Select the required GCC 13.3, independently of Studio's legacy PATH."""
    explicit = environment.get('RTT_EXEC_PATH')
    candidates = [explicit] if explicit else []
    # Studio's project generator exports its default GCC 5.4.1 as RTT_EXEC_PATH.
    # Prefer a compatible explicit path, but keep searching when it is unsuitable.
    for directory in studio_directories(environment, project_dir):
        candidates.append(os.path.join(directory, 'repo', 'Extract',
            'ToolChain_Support_Packages', 'ARM',
            'GNU_Tools_for_ARM_Embedded_Processors', '13.3', 'bin'))
    candidates.extend(environment.get('PATH', '').split(os.pathsep))

    seen = set()
    rejected = []
    compiler = 'arm-none-eabi-gcc.exe' if os.name == 'nt' else 'arm-none-eabi-gcc'
    for candidate in candidates:
        directory = os.path.abspath(candidate.strip('"'))
        if directory in seen:
            continue
        seen.add(directory)
        executable = os.path.join(directory, compiler)
        if not os.path.isfile(executable):
            continue
        try:
            version = subprocess.check_output([executable, '-dumpversion'],
                stderr=subprocess.STDOUT, env=environment).decode('ascii').strip()
        except (OSError, subprocess.CalledProcessError, UnicodeError):
            rejected.append(executable + ' (version check failed)')
            continue
        if version.split('.')[:2] == ['13', '3']:
            return directory
        rejected.append(executable + ' (' + version + ')')
    detail = ' Found: ' + '; '.join(rejected) if rejected else ''
    raise RuntimeError('OpenMV requires ARM GCC 13.3.Rel1. Set RTT_EXEC_PATH '
                       'to its bin directory.' + detail)


def launch(arguments, cwd=None):
    environment = os.environ.copy()
    # Studio sets PYTHONPATH to Python27; do not load those modules in Python 3.
    for name in ('PYTHONHOME', 'PYTHONPATH', 'SCONS_LIB_DIR'):
        environment.pop(name, None)
    probe = ('import sys, json; '
             'sys.exit(1) if sys.version_info < (3, 10) else None; '
             'import SCons; print(json.dumps(sys.executable))')
    seen = set()
    executable = None
    for command in python_commands(environment):
        key = tuple(command)
        if key in seen:
            continue
        seen.add(key)
        try:
            output = subprocess.check_output(command + ['-c', probe],
                                             stderr=subprocess.STDOUT, env=environment)
            executable = json.loads(output.decode('ascii').strip())
            # Python 2's Windows subprocess API rejects unicode env values.
            if sys.version_info[0] == 2:
                executable = executable.encode(sys.getfilesystemencoding())
            break
        except (OSError, subprocess.CalledProcessError):
            continue
    if not executable:
        print('OpenMV requires Python 3.10+ with SCons. Set TITAN_OPENMV_PYTHON '
              'to its python.exe and install SCons with: python -m pip install scons',
              file=sys.stderr)
        return 1

    environment['PATH'] = os.path.dirname(executable) + os.pathsep + environment.get('PATH', '')
    print('OpenMV SCons: ' + executable)
    sys.stdout.flush()
    return subprocess.call([executable, '-m', 'SCons'] + list(arguments),
                           cwd=cwd, env=environment)


if __name__ == '__main__':
    sys.exit(launch(sys.argv[1:]))
