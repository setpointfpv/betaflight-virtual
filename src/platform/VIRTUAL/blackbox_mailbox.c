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

// The "virtual" blackbox device, as SITL has, writing to the mailbox instead
// of a file: the host receives an ordinary .bbl byte stream.

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "blackbox/blackbox_virtual.h"

#include "virtual_mailbox.h"

static bool logOpen;
static uint32_t logNumber;

bool blackboxVirtualOpen(void)
{
    return true;
}

void blackboxVirtualPutChar(uint8_t value)
{
    if (logOpen) {
        MBX_REG(MBX_BLACKBOX_DATA) = value;
    }
}

void blackboxVirtualWrite(const uint8_t *buffer, uint32_t len)
{
    if (logOpen) {
        while (len--) {
            MBX_REG(MBX_BLACKBOX_DATA) = *buffer++;
        }
    }
}

bool blackboxVirtualFlush(void)
{
    return logOpen;
}

bool blackboxVirtualBeginLog(void)
{
    if (logOpen) {
        return false;
    }
    MBX_REG(MBX_BLACKBOX_CONTROL) = 1;
    logOpen = true;
    logNumber++;
    return true;
}

bool blackboxVirtualEndLog(void)
{
    if (logOpen) {
        MBX_REG(MBX_BLACKBOX_CONTROL) = 2;
        logOpen = false;
    }
    return true;
}

void blackboxVirtualClose(void)
{
    blackboxVirtualEndLog();
}

uint32_t blackboxVirtualLogFileNumber(void)
{
    return logNumber;
}
