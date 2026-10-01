# The virtual board: a plain Cortex-M4F with invented peripherals, run by a
# host emulator rather than silicon. Built with the normal ARM toolchain and
# linker-script machinery so parameter groups work exactly as on hardware.
#
# 4.5 has no platform directories: the board's code stays in
# src/platform/VIRTUAL, as on later releases, and is reached from here.

VIRTUAL_DIR := $(ROOT)/src/platform/VIRTUAL

INCLUDE_DIRS := \
        $(INCLUDE_DIRS) \
        $(VIRTUAL_DIR) \
        $(VIRTUAL_DIR)/include \
        $(CMSIS_DIR)/Core/Include

VPATH := $(VPATH):$(ROOT)/src/platform

MCU_COMMON_SRC  := \
        VIRTUAL/startup_virtual.c \
        VIRTUAL/virtual.c \
        VIRTUAL/serial_virtual.c \
        VIRTUAL/blackbox_mailbox.c

# As an F4 on 4.5, including single-precision constants: a bare 0.5 is a
# float, which changes the arithmetic, not just the warnings.
ARCH_FLAGS      = -mthumb -mcpu=cortex-m4 -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant
DEVICE_FLAGS    = -DVIRTUAL_BOARD
LD_SCRIPT       = $(VIRTUAL_DIR)/link/virtual_m4.ld
STARTUP_SRC     =

MCU_FLASH_SIZE  := 2048

# target.mk sets SIMULATOR_BUILD for its hardware-free source list, but the
# define that comes with it also switches SITL behaviour on in the flight
# code (attitude from the simulator, TCP serial). The virtual board runs the
# flight code as hardware does, so the define is cancelled after the fact:
# 4.5 has no way to drop a flag, and -U after -D wins.
override EXTRA_FLAGS += -USIMULATOR_BUILD

# 4.5 builds the hardware layer (system, DShot output) for every target; the
# board's own versions are in virtual.c.
MCU_EXCLUDES = \
        drivers/system.c \
        drivers/dshot_dpwm.c \
        drivers/accgyro/accgyro_virtual.c \
        fc/hardfaults.c \
        drivers/serial_tcp.c \
        drivers/display_ug2864hsweg01.c \
        io/displayport_oled.c

# CMSIS-DSP for the dynamic notch's FFT, built exactly as for an F4.
DSP_LIB := $(CMSIS_DIR)/DSP
DEVICE_FLAGS += -DARM_MATH_MATRIX_CHECK -DARM_MATH_ROUNDING -D__FPU_PRESENT=1 -DUNALIGNED_SUPPORT_DISABLE -DARM_MATH_CM4
