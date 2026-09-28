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

// The virtual board: a Cortex-M4F core with one mailbox peripheral and no
// real pins, buses or DMA. Unlike SITL it is not a simulator build: every
// flight-time check and behaviour of real hardware stays in.

#pragma once

// Core intrinsics only (BASEPRI, barriers): there is no vendor device header.
#include "cmsis_compiler.h"

#define IOCFG_OUT_PP        0
#define IOCFG_OUT_OD        0
#define IOCFG_AF_PP         0
#define IOCFG_AF_OD         0
#define IOCFG_IPD           0
#define IOCFG_IPU           0
#define IOCFG_IN_FLOATING   0

#define SPIDEV_COUNT        0
#define I2CDEV_COUNT        0
#define SERIAL_TRAIT_PIN_CONFIG 0

#define GYRO_COUNT 1

typedef void* ADC_TypeDef;

// drivers/nvic.h builds priorities from the grouping, as on an F4.
#define NVIC_PriorityGroup_2 0x500

// Kept across a reset, like backup RAM: holds the config storage.
#define PERSISTENT          __attribute__ ((section(".persistent_data"), aligned(4)))

// As an F4/F7/H7: an 8 kHz gyro and PID loop by default.
#define TASK_GYROPID_DESIRED_PERIOD     125
#define SCHEDULER_DELAY_LIMIT           10

// The filters of an F4/F7, which are what the board exists to run.
#define USE_RPM_FILTER
#define USE_DYN_IDLE
#define USE_DYN_NOTCH_FILTER
