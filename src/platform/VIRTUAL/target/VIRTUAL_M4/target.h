/*
 * This file is part of Betaflight.
 *
 * Betaflight is free software. You can redistribute this software
 * and/or modify this software under the terms of the GNU General
 * Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later
 * version.
 *
 * Betaflight is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

// A Cortex-M4F flight controller that exists only in an emulator. The host
// supplies time, gyro and stick input through one mailbox peripheral, lets the
// firmware run until it idles, and reads the motor outputs back. The firmware
// is otherwise built and behaves as it would on an F4.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common/utils.h"

#define TARGET_BOARD_IDENTIFIER "VIRT"
#define USBD_PRODUCT_STRING     "Betaflight Virtual"

#define SYSTEM_HSE_MHZ 0
#define DEFAULT_CPU_OVERCLOCK 0

#define DMA_RAM
#define DMA_RW_AXI
#define DMA_RAM_R
#define DMA_RAM_W
#define DMA_RAM_RW
#define DMA_DATA_ZERO_INIT
#define DMA_DATA
#define STATIC_DMA_DATA_AUTO

// Config storage is plain RAM kept across resets; the host saves and
// restores it as the flash image.
#define CONFIG_IN_RAM
#define EEPROM_SIZE     32768

#define U_ID_0 0
#define U_ID_1 1
#define U_ID_2 2

#define USE_GYRO
#define USE_VIRTUAL_GYRO
#define USE_ACC
#define USE_VIRTUAL_ACC
#define VIRTUAL_GYRO_SAMPLE_RATE_HZ 8000

#undef USE_MAG
#undef USE_BARO
#undef USE_GPS
#undef USE_RANGEFINDER

// Motor outputs go to the mailbox whatever the protocol; DShot keeps the
// mixer's output range and idle exactly as on a DShot quad.
#define USE_PWM_OUTPUT
#undef USE_DSHOT_BITBANG
#define USABLE_TIMER_CHANNEL_COUNT 0

// Stick input arrives as whole RC frames posted by the host.
#define ENABLE_RX_UDP           1

// MSP and the CLI over one byte stream, presented as the USB VCP.
#define USE_VCP

#define USE_PARAMETER_GROUPS

#undef USE_STACK_CHECK
#undef USE_DASHBOARD
#undef USE_ADC
#undef USE_OSD
#undef USE_CMS
#undef USE_LED_STRIP
#undef USE_TRANSPONDER
#undef USE_RX_PPM
#undef USE_RX_PWM
#undef USE_SERIALRX
#undef USE_SERIALRX_CRSF
#undef USE_SERIALRX_GHST
#undef USE_SERIALRX_IBUS
#undef USE_SERIALRX_SBUS
#undef USE_SERIALRX_SPEKTRUM
#undef USE_SERIALRX_SUMD
#undef USE_SERIALRX_SUMH
#undef USE_SERIALRX_XBUS
#undef USE_SERIALRX_JETIEXBUS
#undef USE_SERIALRX_FPORT
#undef USE_SERIALRX_SRXL2
#undef USE_TELEMETRY
#undef USE_TELEMETRY_LTM
#undef USE_TELEMETRY_FRSKY_HUB
#undef USE_TELEMETRY_HOTT
#undef USE_TELEMETRY_SMARTPORT
#undef USE_TELEMETRY_CRSF
#undef USE_TELEMETRY_GHST
#undef USE_TELEMETRY_IBUS
#undef USE_TELEMETRY_JETIEXBUS
#undef USE_TELEMETRY_SRXL
#undef USE_TELEMETRY_MAVLINK
#undef USE_RESOURCE_MGMT
#undef USE_VTX_COMMON
#undef USE_VTX_CONTROL
#undef USE_VTX_SMARTAUDIO
#undef USE_VTX_TRAMP
#undef USE_VTX_MSP
#undef USE_VTX_RTC6705
#undef USE_VTX_RTC6705_SOFTSPI
#undef USE_RX_SPI
#undef USE_RX_EXPRESSLRS
#undef USE_RX_CC2500
#undef USE_CAMERA_CONTROL
#undef USE_SERIAL_4WAY_BLHELI_BOOTLOADER
#undef USE_SERIAL_4WAY_SK_BOOTLOADER
#undef USE_SERIAL_PASSTHROUGH
#undef USE_GYRO_REGISTER_DUMP
#undef USE_BEEPER
#undef USE_I2C
#undef USE_SPI
#undef USE_UART
#undef USE_SOFTSERIAL
#undef USE_FLASH
#undef USE_SDCARD
#undef USE_BLACKBOX


// No real sensors, flash chips, pins or OSD.
#undef USE_ACC_MPU6050
#undef USE_ACC_MPU6500
#undef USE_ACC_SPI_ICM20689
#undef USE_ACC_SPI_ICM42605
#undef USE_ACC_SPI_ICM42688P
#undef USE_ACC_SPI_MPU6000
#undef USE_ACC_SPI_MPU6500
#undef USE_ACCGYRO_BMI160
#undef USE_ACCGYRO_BMI270
#undef USE_ACCGYRO_ICM40609D
#undef USE_ACCGYRO_ICM42622P
#undef USE_ACCGYRO_ICM42686P
#undef USE_ACCGYRO_ICM45605
#undef USE_ACCGYRO_ICM45686
#undef USE_ACCGYRO_IIM42652
#undef USE_ACCGYRO_IIM42653
#undef USE_ACCGYRO_LSM6DSK320X
#undef USE_ACCGYRO_LSM6DSO
#undef USE_ACCGYRO_LSM6DSV16X
#undef USE_GYRO_MPU6050
#undef USE_GYRO_MPU6500
#undef USE_GYRO_SPI_ICM20689
#undef USE_GYRO_SPI_ICM42605
#undef USE_GYRO_SPI_ICM42688P
#undef USE_GYRO_SPI_MPU6000
#undef USE_GYRO_SPI_MPU6500
#undef USE_FLASH
#undef USE_FLASHFS
#undef USE_FLASH_TOOLS
#undef USE_FLASH_CHIP
#undef USE_FLASH_M25P16
#undef USE_FLASH_W25N01G
#undef USE_FLASH_W25N02K
#undef USE_FLASH_W25M
#undef USE_FLASH_W25M512
#undef USE_FLASH_W25M02G
#undef USE_FLASH_W25Q128FV
#undef USE_FLASH_PY25Q128HA
#undef USE_FLASH_MX66UW1G45G
#undef USE_PINIO
#undef USE_PINIOBOX
#undef USE_MAX7456
#undef USE_OSD_SD
#undef USE_OSD_HD
#undef USE_FRSKYOSD
#undef USE_OSD_CUSTOM_TEXT
#undef ENABLE_OSD_CUSTOM_TEXT
#define ENABLE_OSD_CUSTOM_TEXT 0

#define TARGET_FLASH_SIZE 2048
#define DEFIO_NO_PORTS
#define FLASH_PAGE_SIZE (0x400)

extern uint32_t SystemCoreClock;

typedef enum
{
    Mode_TEST = 0x0,
    Mode_Out_PP = 0x10
} GPIO_Mode;

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;
typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
typedef enum {TEST_IRQ = 0 } IRQn_Type;

typedef struct {
    void* test;
} DMA_TypeDef;

typedef struct {
    void* test;
} DMA_Channel_TypeDef;

typedef struct {
    void* test;
} DMA_InitTypeDef;

struct spiResource_s;
struct quadSpiResource_s;
struct octoSpiResource_s;
struct i2cResource_s;
#define USE_SCHEDULER_IDLE_HOOK
