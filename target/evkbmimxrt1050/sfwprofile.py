import os
import sys

# toolchains options
ARCH='arm'
CPU='cortex-m7'
CROSS_TOOL='gcc'

if os.getenv('SFW_CC'):
    CROSS_TOOL = os.getenv('SFW_CC')
if os.getenv('SFW_ROOT'):
    SFW_ROOT = os.getenv('SFW_ROOT')

# cross_tool provides the cross compiler
# EXEC_PATH is the compiler execute path, for example,  gcc, keil, IAR
if  CROSS_TOOL == 'gcc':
    PLATFORM    = 'gcc'
    EXEC_PATH   = r'/opt/share/toolchain/gcc-arm-none-eabi-9-2019-q4-major/bin'

if os.getenv('SFW_EXEC_PATH'):
    EXEC_PATH = os.getenv('SFW_EXEC_PATH')

#BUILD = 'debug'
BUILD = 'release'

MV = 'mv'

if PLATFORM == 'gcc':
    PREFIX = 'arm-none-eabi-'
    CC = PREFIX + 'gcc'
    CPP = PREFIX + 'cpp'
    CXX = PREFIX + 'g++'
    AS = PREFIX + 'gcc'
    AR = PREFIX + 'ar'
    LINK = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY = PREFIX + 'objcopy'
    STRIP = PREFIX + 'strip'

    DEVICE = ' -mcpu=' + CPU + ' -mthumb -specs=nosys.specs -mfpu=fpv5-d16 -mfloat-abi=hard -ffunction-sections -fdata-sections '
    CFLAGS = DEVICE + ' -Wall -D__FPU_PRESENT'
    AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp -Wa,-mimplicit-it=thumb '
    LFLAGS = DEVICE + ' -lm -lgcc -lc' + ' -Wl,--gc-sections,-Map=build/sfw.map,-cref,-u,Reset_Handler -T board/link/.MIMXRT1052xxxxx_flexspi_nor_cpp.ld'
    PRE_ACTION = CPP + ' -E -P -I . board/link/MIMXRT1052xxxxx_flexspi_nor.ld' + ' -O board/link/.MIMXRT1052xxxxx_flexspi_nor_cpp.ld\n'

    CPATH = ''
    LPATH = ''

    if BUILD == 'debug':
        CFLAGS += ' -gdwarf-2'
        AFLAGS += ' -gdwarf-2'
        CFLAGS += ' -O0'
    else:
        CFLAGS += ' -O2 -Os'

    POST_ACTION = OBJCPY + ' -O binary $TARGET build/sfw.bin\n' + SIZE + ' $TARGET \n'
    POST_ACTION += MV + ' $TARGET build/$TARGET\n'

    # module setting 
    CXXFLAGS = ' -Woverloaded-virtual -fno-exceptions -fno-rtti '
    M_CFLAGS = CFLAGS + ' -mlong-calls -fPIC '
    M_CXXFLAGS = CXXFLAGS + ' -mlong-calls -fPIC'
    M_LFLAGS = DEVICE + CXXFLAGS + ' -Wl,--gc-sections,-z,max-page-size=0x4' +\
                                    ' -shared -fPIC -nostartfiles -static-libgcc'
    M_POST_ACTION = STRIP + ' -R .hash $TARGET\n' + SIZE + ' $TARGET \n'