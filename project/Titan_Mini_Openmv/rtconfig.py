"""GCC 13.3 configuration for the Titan Mini Cortex-M85 OpenMV port."""
import os
import shutil

ARCH = 'arm'
CPU = 'cortex-m85'
CROSS_TOOL = os.getenv('RTT_CC', 'gcc')
if CROSS_TOOL != 'gcc':
    raise RuntimeError('Titan Mini OpenMV currently supports GCC; set RTT_CC=gcc')
PLATFORM = 'gcc'
EXEC_PATH = os.getenv('RTT_EXEC_PATH')
if not EXEC_PATH:
    compiler = shutil.which('arm-none-eabi-gcc')
    if not compiler:
        raise RuntimeError('ARM GCC not found; add it to PATH or set RTT_EXEC_PATH to its bin directory')
    EXEC_PATH = os.path.dirname(compiler)
BUILD = os.getenv('OPENMV_BUILD', 'release')
PREFIX = 'arm-none-eabi-'
CC = AS = LINK = PREFIX + 'gcc'
CXX = PREFIX + 'g++'
AR = PREFIX + 'ar'
SIZE = PREFIX + 'size'
OBJDUMP = PREFIX + 'objdump'
OBJCPY = PREFIX + 'objcopy'
NM = PREFIX + 'nm'
TARGET_EXT = 'elf'
DEVICE = ' -mcpu=cortex-m85 -mthumb -mfloat-abi=hard -ffunction-sections -fdata-sections'
CFLAGS = DEVICE + ' -Dgcc -std=gnu11 -include platform/include/compiler.h'
CFLAGS += ' -Os -g3' if BUILD == 'debug' else ' -Os'
AFLAGS = DEVICE + ' -c -x assembler-with-cpp -Wa,-mimplicit-it=thumb'
CXXFLAGS = CFLAGS.replace('-std=gnu11', '-std=c++17') + ' -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit'
LFLAGS = DEVICE + ' -nostartfiles --specs=nano.specs --specs=nosys.specs'
LFLAGS += ' -Wl,--gc-sections,-Map=build/firmware/rtthread.map,-cref,-u,Reset_Handler -T board/linker_scripts/fsp.ld'
CPATH = LPATH = ''
POST_ACTION = ''
