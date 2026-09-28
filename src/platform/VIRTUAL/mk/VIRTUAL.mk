# The virtual board: a plain Cortex-M4F with invented peripherals, run by a
# host emulator rather than silicon. Built with the normal ARM toolchain and
# linker-script machinery so parameter groups work exactly as on hardware.

PLATFORM_SDK := none

INCLUDE_DIRS := \
        $(INCLUDE_DIRS) \
        $(TARGET_PLATFORM_DIR) \
        $(TARGET_PLATFORM_DIR)/include \
        $(LIB_MAIN_DIR)/CMSIS/Core/Include

MCU_COMMON_SRC  := \
        VIRTUAL/startup_virtual.c \
        VIRTUAL/virtual.c \
        VIRTUAL/serial_virtual.c \
        VIRTUAL/blackbox_mailbox.c

ARCH_FLAGS      = -mthumb -mcpu=cortex-m4 -march=armv7e-m -mfloat-abi=hard -mfpu=fpv4-sp-d16
DEVICE_FLAGS    = -DVIRTUAL_BOARD
LD_SCRIPT       = $(LINKER_DIR)/virtual_m4.ld
STARTUP_SRC     =

MCU_FLASH_SIZE  := 2048

MCU_EXCLUDES = \
        drivers/accgyro/accgyro_virtual.c \
        drivers/serial_tcp.c \
        drivers/display_ug2864hsweg01.c \
        io/displayport_oled.c

# CMSIS-DSP for the dynamic notch's FFT, built exactly as for an F4.
DSP_LIB := $(LIB_MAIN_DIR)/CMSIS/DSP
DEVICE_FLAGS += -DARM_MATH_MATRIX_CHECK -DARM_MATH_ROUNDING -D__FPU_PRESENT=1 -DUNALIGNED_SUPPORT_DISABLE -DARM_MATH_CM4
