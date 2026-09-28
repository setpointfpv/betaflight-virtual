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

// The virtual board's system layer: time, sleep, sensors, stick input and
// motors, each a few mailbox registers. Time belongs to the host and stands
// still while the firmware runs; whenever the firmware has nothing to do it
// says until when, and sleeps.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "build/build_config.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_virtual.h"
#include "drivers/dshot.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/light_led.h"
#include "drivers/motor_impl.h"
#include "drivers/pwm_output.h"
#include "drivers/pwm_output_impl.h"
#include "drivers/sound_beeper.h"
#include "drivers/system.h"
#include "drivers/time.h"
#include "drivers/usb_io.h"

#include "pg/motor.h"
#include "rx/rx.h"
#include "rx/msp.h"
#include "scheduler/scheduler.h"

#include "virtual_mailbox.h"

uint32_t SystemCoreClock;
static uint32_t cyclesPerMicro;

// --- System

void systemInit(void)
{
    MBX_REG(MBX_GUEST_ABI) = VIRTUAL_MAILBOX_ABI;
    MBX_REG(MBX_STAGE) = VIRTUAL_STAGE_SYSTEM_INIT;
    SystemCoreClock = MBX_REG(MBX_CLOCK_HZ);
    if (SystemCoreClock < 1000000) {
        SystemCoreClock = 100000000;
    }
    cyclesPerMicro = SystemCoreClock / 1000000;
}

void systemReset(void)
{
    MBX_REG(MBX_RESET) = 1;
    while (1) {
        __asm__ volatile ("wfi");
    }
}

void systemResetToBootloader(bootloaderRequestType_e requestType)
{
    UNUSED(requestType);
    systemReset();
}

void systemResetWithoutDisablingCaches(void)
{
    systemReset();
}

bool isMPUSoftReset(void)
{
    return false;
}

void systemProcessResetReason(void)
{
}

void cycleCounterInit(void)
{
}

void timerInit(void)
{
}

void failureMode(failureMode_e mode)
{
    MBX_REG(MBX_FAULT) = 0x100 + mode;
    while (1) {
        __asm__ volatile ("wfi");
    }
}

void indicateFailure(failureMode_e mode, int repeatCount)
{
    UNUSED(mode);
    UNUSED(repeatCount);
}

// --- Time

uint32_t getCycleCounter(void)
{
    return MBX_REG(MBX_CYCLES);
}

timeUs_t micros(void)
{
    return MBX_REG(MBX_TIME_US_LO);
}

timeUs_t microsISR(void)
{
    return micros();
}

timeMs_t millis(void)
{
    const uint64_t us = ((uint64_t)MBX_REG(MBX_TIME_US_HI) << 32) | MBX_REG(MBX_TIME_US_LO);
    return (timeMs_t)(us / 1000);
}

int32_t clockCyclesToMicros(int32_t clockCycles)
{
    return clockCycles / (int32_t)cyclesPerMicro;
}

float clockCyclesToMicrosf(int32_t clockCycles)
{
    return (float)clockCycles / cyclesPerMicro;
}

int32_t clockCyclesTo10thMicros(int32_t clockCycles)
{
    return 10 * clockCycles / (int32_t)cyclesPerMicro;
}

int32_t clockCyclesTo100thMicros(int32_t clockCycles)
{
    return 100 * clockCycles / (int32_t)cyclesPerMicro;
}

uint32_t clockMicrosToCycles(uint32_t us)
{
    return us * cyclesPerMicro;
}

// Time only moves while the firmware sleeps, so every wait is a sleep.
static void sleepUntilCycles(uint32_t cycles)
{
    MBX_REG(MBX_IDLE_UNTIL) = cycles;
    __asm__ volatile ("wfi");
}

void schedulerIdle(uint32_t untilCycles)
{
    MBX_REG(MBX_STAGE) = VIRTUAL_STAGE_RUNNING;
    sleepUntilCycles(untilCycles);
}

void delayMicroseconds(timeUs_t us)
{
    const uint32_t until = getCycleCounter() + clockMicrosToCycles(us);
    while (cmp32(getCycleCounter(), until) < 0) {
        sleepUntilCycles(until);
    }
}

void delay(timeMs_t ms)
{
    while (ms--) {
        delayMicroseconds(1000);
    }
}

// --- Stick input

static uint32_t lastRcSeq;

static void pollRc(void)
{
    const uint32_t seq = MBX_REG(MBX_RC_SEQ);
    if (seq == lastRcSeq) {
        return;
    }
    lastRcSeq = seq;
    uint16_t channels[16];
    const uint32_t count = MIN(MBX_REG(MBX_RC_COUNT), ARRAYLEN(channels));
    for (unsigned i = 0; i < count; i++) {
        channels[i] = (uint16_t)MBX_REG(MBX_RC_BASE + 4 * i);
    }
    // 2025.12 has no UDP receiver for the board to feed, so the frame goes in
    // as an MSP RC frame would: one complete frame per host frame.
    rxMspFrameReceive(channels, (int)count);
}

// --- Gyro and accelerometer: the sample the host posted for this time

gyroDev_t *virtualGyroDev;
accDev_t *virtualAccDev;
static uint32_t lastGyroSeq;
static uint32_t lastAccSeq;

static void virtualGyroInit(gyroDev_t *gyro)
{
    virtualGyroDev = gyro;
}

static bool virtualGyroRead(gyroDev_t *gyro)
{
    // Stick input is checked here because the gyro task runs once per posted
    // sample, just before the scheduler looks for a new RC frame.
    pollRc();

    const uint32_t seq = MBX_REG(MBX_SENSOR_SEQ);
    if (seq == lastGyroSeq) {
        return false;
    }
    lastGyroSeq = seq;
    gyro->gyroADCRaw[X] = (int16_t)MBX_REG(MBX_GYRO_X);
    gyro->gyroADCRaw[Y] = (int16_t)MBX_REG(MBX_GYRO_Y);
    gyro->gyroADCRaw[Z] = (int16_t)MBX_REG(MBX_GYRO_Z);
    return true;
}

static bool virtualGyroReadTemperature(gyroDev_t *gyro, int16_t *temperatureData)
{
    UNUSED(gyro);
    *temperatureData = 25;
    return true;
}

bool virtualGyroDetect(gyroDev_t *gyro)
{
    gyro->initFn = virtualGyroInit;
    gyro->readFn = virtualGyroRead;
    gyro->temperatureFn = virtualGyroReadTemperature;
    gyro->scale = GYRO_SCALE_2000DPS;
    return true;
}

void virtualGyroSet(gyroDev_t *gyro, int16_t x, int16_t y, int16_t z)
{
    UNUSED(gyro);
    UNUSED(x);
    UNUSED(y);
    UNUSED(z);
}

static void virtualAccInit(accDev_t *acc)
{
    virtualAccDev = acc;
}

static bool virtualAccRead(accDev_t *acc)
{
    const uint32_t seq = MBX_REG(MBX_SENSOR_SEQ);
    if (seq == lastAccSeq) {
        return false;
    }
    lastAccSeq = seq;
    acc->ADCRaw[X] = (int16_t)MBX_REG(MBX_ACC_X);
    acc->ADCRaw[Y] = (int16_t)MBX_REG(MBX_ACC_Y);
    acc->ADCRaw[Z] = (int16_t)MBX_REG(MBX_ACC_Z);
    return true;
}

bool virtualAccDetect(accDev_t *acc)
{
    acc->initFn = virtualAccInit;
    acc->readFn = virtualAccRead;
    acc->revisionCode = 0;
    return true;
}

void virtualAccSet(accDev_t *acc, int16_t x, int16_t y, int16_t z)
{
    UNUSED(acc);
    UNUSED(x);
    UNUSED(y);
    UNUSED(z);
}

// --- Motors: whatever the mixer writes, as floats in the protocol's own units

static bool motorsEnabled;
static uint32_t motorSeq;

static bool virtualMotorEnable(void)
{
    motorsEnabled = true;
    return true;
}

static void virtualMotorDisable(void)
{
    motorsEnabled = false;
}

static bool virtualMotorIsEnabled(unsigned index)
{
    UNUSED(index);
    return motorsEnabled;
}

static void virtualMotorWrite(uint8_t index, float value)
{
    if (index < 8) {
        union { float f; uint32_t u; } bits = { .f = value };
        MBX_REG(MBX_MOTOR_BASE + 4 * index) = bits.u;
    }
}

static void virtualMotorWriteInt(uint8_t index, uint16_t value)
{
    virtualMotorWrite(index, (float)value);
}

static void virtualMotorUpdateComplete(void)
{
    MBX_REG(MBX_MOTOR_SEQ) = ++motorSeq;
}

static void virtualMotorShutdown(void)
{
    motorsEnabled = false;
}

static float analogConvertFromExternal(uint16_t externalValue)
{
    return (float)externalValue;
}

static uint16_t analogConvertToExternal(float motorValue)
{
    return (uint16_t)motorValue;
}

#ifdef USE_DSHOT
FAST_DATA_ZERO_INIT bool useDshotTelemetry = false;
FAST_DATA_ZERO_INIT dshotTelemetryCycleCounters_t dshotDMAHandlerCycleCounters;

// Bidirectional DShot: the host supplies each motor's speed, which is encoded
// as an ESC would send it (electrical period in microseconds, 3-bit exponent
// and 9-bit mantissa) so the firmware's own decoder sees the same resolution.
static uint16_t encodeErpmTelemetry(uint32_t erpm100)
{
    if (erpm100 == 0) {
        return 0x0fff;
    }
    uint32_t period = (1000000 * 60 / 100 + erpm100 / 2) / erpm100;
    if (period == 0) {
        period = 1;
    }
    unsigned exponent = 0;
    while ((period >> exponent) > 0x1ff && exponent < 7) {
        exponent++;
    }
    return (uint16_t)((exponent << 9) | ((period >> exponent) & 0x1ff));
}

static bool virtualDecodeTelemetry(void)
{
    for (unsigned i = 0; i < dshotMotorCount && i < 8; i++) {
        dshotTelemetryState.motorState[i].rawValue = encodeErpmTelemetry(MBX_REG(MBX_ERPM_BASE + 4 * i));
    }
    dshotTelemetryState.rawValueState = DSHOT_RAW_VALUE_STATE_NOT_PROCESSED;
    return true;
}

static const motorVTable_t dshotVTable = {
    .postInit = motorPostInitNull,
    .convertExternalToMotor = dshotConvertFromExternal,
    .convertMotorToExternal = dshotConvertToExternal,
    .enable = virtualMotorEnable,
    .disable = virtualMotorDisable,
    .isMotorEnabled = virtualMotorIsEnabled,
    .decodeTelemetry = virtualDecodeTelemetry,
    .write = virtualMotorWrite,
    .writeInt = virtualMotorWriteInt,
    .updateComplete = virtualMotorUpdateComplete,
    .shutdown = virtualMotorShutdown,
};

bool dshotPwmDevInit(motorDevice_t *device, const motorDevConfig_t *motorConfig)
{
    device->vTable = &dshotVTable;
    useDshotTelemetry = motorConfig->useDshotTelemetry;
    dshotMotorCount = device->count;
    MBX_REG(MBX_MOTOR_COUNT) = device->count;
    return true;
}
#endif

static const motorVTable_t analogVTable = {
    .postInit = motorPostInitNull,
    .convertExternalToMotor = analogConvertFromExternal,
    .convertMotorToExternal = analogConvertToExternal,
    .enable = virtualMotorEnable,
    .disable = virtualMotorDisable,
    .isMotorEnabled = virtualMotorIsEnabled,
    .decodeTelemetry = motorDecodeTelemetryNull,
    .write = virtualMotorWrite,
    .writeInt = virtualMotorWriteInt,
    .updateComplete = virtualMotorUpdateComplete,
    .shutdown = virtualMotorShutdown,
};

bool motorPwmDevInit(motorDevice_t *device, const motorDevConfig_t *motorConfig, uint16_t idlePulse)
{
    UNUSED(motorConfig);
    UNUSED(idlePulse);
    device->vTable = &analogVTable;
    MBX_REG(MBX_MOTOR_COUNT) = device->count;
    return true;
}

// --- No pins, no EXTI, no USB cable

void IOConfigGPIO(IO_t io, ioConfig_t cfg)
{
    UNUSED(io);
    UNUSED(cfg);
}

void IOHi(IO_t io)
{
    UNUSED(io);
}

void IOLo(IO_t io)
{
    UNUSED(io);
}

bool IORead(IO_t io)
{
    UNUSED(io);
    return false;
}

void IOWrite(IO_t io, bool value)
{
    UNUSED(io);
    UNUSED(value);
}

void IOToggle(IO_t io)
{
    UNUSED(io);
}

void IOInitGlobal(void)
{
}

IO_t IOGetByTag(ioTag_t tag)
{
    UNUSED(tag);
    return NULL;
}

void EXTIInit(void)
{
}

void EXTIHandlerInit(extiCallbackRec_t *cb, extiHandlerCallback *fn)
{
    UNUSED(cb);
    UNUSED(fn);
}

void EXTIConfig(IO_t io, extiCallbackRec_t *cb, int irqPriority, ioConfig_t config, extiTrigger_t trigger)
{
    UNUSED(io);
    UNUSED(cb);
    UNUSED(irqPriority);
    UNUSED(config);
    UNUSED(trigger);
}

void EXTIEnable(IO_t io)
{
    UNUSED(io);
}

// --- Servos: none on a quad

void servoDevInit(const servoDevConfig_t *servoConfig)
{
    UNUSED(servoConfig);
}

void pwmWriteServo(uint8_t index, float value)
{
    UNUSED(index);
    UNUSED(value);
}

void unusedPinsInit(void)
{
}

void debugInit(void)
{
}

bool usbCableIsInserted(void)
{
    return true;
}

bool usbCableIsActive(void)
{
    return true;
}

// 2025.12 asks the platform what MCU it is for the status and MSP replies.
// There is no type for this board; it reports as the simulator does.
const mcuTypeInfo_t *getMcuTypeInfo(void)
{
    static const mcuTypeInfo_t info = { .id = MCU_TYPE_SIMULATOR, .name = "VIRTUAL_M4" };
    return &info;
}
