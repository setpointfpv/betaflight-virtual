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

// The mailbox byte stream, presented as the USB VCP so MSP and the CLI find
// it exactly where they do on a real board.

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "common/utils.h"
#include "drivers/serial.h"
#include "drivers/serial_usb_vcp.h"

#include "virtual_mailbox.h"

static vcpPort_t vcpPort;

static void vcpSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    UNUSED(instance);
    UNUSED(baudRate);
}

static void vcpSetMode(serialPort_t *instance, portMode_e mode)
{
    UNUSED(instance);
    UNUSED(mode);
}

static void vcpSetCtrlLineStateCb(serialPort_t *instance, void (*cb)(void *context, uint16_t ctrlLineState), void *context)
{
    UNUSED(instance);
    UNUSED(cb);
    UNUSED(context);
}

static void vcpSetBaudRateCb(serialPort_t *instance, void (*cb)(serialPort_t *context, uint32_t baud), serialPort_t *context)
{
    UNUSED(instance);
    UNUSED(cb);
    UNUSED(context);
}

static bool isUsbVcpTransmitBufferEmpty(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;
}

static uint32_t usbVcpAvailable(const serialPort_t *instance)
{
    UNUSED(instance);
    return MBX_REG(MBX_SERIAL_RX_COUNT);
}

static uint8_t usbVcpRead(serialPort_t *instance)
{
    UNUSED(instance);
    return (uint8_t)MBX_REG(MBX_SERIAL_RX_DATA);
}

static void usbVcpWrite(serialPort_t *instance, uint8_t c)
{
    UNUSED(instance);
    MBX_REG(MBX_SERIAL_TX_DATA) = c;
}

static void usbVcpWriteBuf(serialPort_t *instance, const void *data, int count)
{
    UNUSED(instance);
    const uint8_t *p = data;
    while (count-- > 0) {
        MBX_REG(MBX_SERIAL_TX_DATA) = *p++;
    }
}

static uint32_t usbTxBytesFree(const serialPort_t *instance)
{
    UNUSED(instance);
    return 4096;
}

static void usbVcpBeginWrite(serialPort_t *instance)
{
    UNUSED(instance);
}

static void usbVcpEndWrite(serialPort_t *instance)
{
    UNUSED(instance);
}

static const struct serialPortVTable usbVTable[] = {
    {
        .serialWrite = usbVcpWrite,
        .serialTotalRxWaiting = usbVcpAvailable,
        .serialTotalTxFree = usbTxBytesFree,
        .serialRead = usbVcpRead,
        .serialSetBaudRate = vcpSetBaudRate,
        .isSerialTransmitBufferEmpty = isUsbVcpTransmitBufferEmpty,
        .setMode = vcpSetMode,
        .setCtrlLineStateCb = vcpSetCtrlLineStateCb,
        .setBaudRateCb = vcpSetBaudRateCb,
        .writeBuf = usbVcpWriteBuf,
        .beginWrite = usbVcpBeginWrite,
        .endWrite = usbVcpEndWrite
    }
};

void usbVcpInit(void)
{
}

serialPort_t *usbVcpOpen(void)
{
    vcpPort_t *s = &vcpPort;
    s->port.vTable = usbVTable;
    return &s->port;
}

uint32_t usbVcpGetBaudRate(serialPort_t *instance)
{
    UNUSED(instance);
    return 115200;
}

uint8_t usbVcpIsConnected(void)
{
    return 1;
}

uint8_t usbVcpIsActive(void)
{
    return 1;
}
