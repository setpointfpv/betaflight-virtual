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

/*
 * The virtual board's only peripheral: one block of 32-bit registers through
 * which a host emulator supplies time, sensors and stick input and reads back
 * motor outputs and a serial byte stream. The layout is the board's ABI and is
 * versioned by VIRTUAL_MAILBOX_ABI; see docs/virtual-board.md.
 */

#pragma once

#include <stdint.h>

#define VIRTUAL_MAILBOX_BASE        0x40000000u
#define VIRTUAL_MAILBOX_ABI         1u
#define VIRTUAL_BOARD_INFO_MAGIC    0x56464331u     // "VFC1"

// Registers, as byte offsets from VIRTUAL_MAILBOX_BASE. RO = the firmware
// only reads it; WO = the firmware only writes it.
#define MBX_MAGIC               0x000   // RO  VIRTUAL_BOARD_INFO_MAGIC
#define MBX_HOST_ABI            0x004   // RO  ABI the host implements
#define MBX_GUEST_ABI           0x008   // WO  ABI this firmware was built for
#define MBX_STAGE               0x00C   // WO  boot progress, see virtualStage_e
#define MBX_TIME_US_LO          0x010   // RO  host time, microseconds
#define MBX_TIME_US_HI          0x014   // RO
#define MBX_CYCLES              0x018   // RO  host time in virtual core cycles
#define MBX_IDLE_UNTIL          0x01C   // WO  cycle count when work is next due; then WFI
#define MBX_CLOCK_HZ            0x020   // RO  virtual core clock
#define MBX_PUTC                0x024   // WO  debug console, one character
#define MBX_RESET               0x028   // WO  firmware asks the host for a reset
#define MBX_FAULT               0x02C   // WO  firmware reports a fault

#define MBX_GYRO_X              0x040   // RO  int16 sensor counts, 16.4 per deg/s
#define MBX_GYRO_Y              0x044
#define MBX_GYRO_Z              0x048
#define MBX_ACC_X               0x04C   // RO  int16 sensor counts, acc_1G per g
#define MBX_ACC_Y               0x050
#define MBX_ACC_Z               0x054
#define MBX_SENSOR_SEQ          0x058   // RO  changes when a new sample is posted

#define MBX_RC_COUNT            0x060   // RO  channels in the current frame
#define MBX_RC_SEQ              0x064   // RO  changes when a new frame is posted
#define MBX_VBAT_MV             0x068   // RO  pack voltage, millivolts
#define MBX_CURRENT_MA          0x06C   // RO  pack current, milliamps
#define MBX_RC_BASE             0x080   // RO  16 channels, microseconds

#define MBX_MOTOR_COUNT         0x0C0   // WO
#define MBX_MOTOR_SEQ           0x0C4   // WO  incremented after each motor update
#define MBX_MOTOR_BASE          0x0D0   // WO  8 float32 motor outputs, as written to the driver

#define MBX_ERPM_BASE           0x100   // RO  8 motor speeds, eRPM / 100 as DShot telemetry reports

#define MBX_SERIAL_RX_COUNT     0x140   // RO  bytes waiting from the host
#define MBX_SERIAL_RX_DATA      0x144   // RO  reading pops one byte
#define MBX_SERIAL_TX_DATA      0x148   // WO  writing sends one byte to the host

#define MBX_BLACKBOX_DATA       0x150   // WO  one byte of blackbox log
#define MBX_BLACKBOX_CONTROL    0x154   // WO  1 begins a log, 2 ends it

#define MBX_SIZE                0x1000

typedef enum {
    VIRTUAL_STAGE_RESET = 0,
    VIRTUAL_STAGE_SYSTEM_INIT = 1,
    VIRTUAL_STAGE_INIT_DONE = 2,
    VIRTUAL_STAGE_RUNNING = 3,
} virtualStage_e;

// Found through vector table entry 7 (reserved on ARMv7-M) so the host can
// check the ABI and find the config storage before running anything.
typedef struct virtualBoardInfo_s {
    uint32_t magic;
    uint32_t abi;
    uint32_t mailboxBase;
    uint32_t eepromAddress;
    uint32_t eepromSize;
    const char *firmwareVersion;
} virtualBoardInfo_t;

#define MBX_REG(offset) (*(volatile uint32_t *)(VIRTUAL_MAILBOX_BASE + (offset)))
