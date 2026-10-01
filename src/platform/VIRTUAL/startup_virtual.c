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

// Vector table and reset for the virtual board. There are no peripheral
// interrupts: the host posts data into the mailbox and wakes the core from WFI.

#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "build/version.h"
#include "config/config_eeprom.h"

#include "virtual_mailbox.h"

extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss;

int main(int argc, char *argv[]);

extern uint8_t eepromData[EEPROM_SIZE];

const virtualBoardInfo_t virtualBoardInfo = {
    .magic = VIRTUAL_BOARD_INFO_MAGIC,
    .abi = VIRTUAL_MAILBOX_ABI,
    .mailboxBase = VIRTUAL_MAILBOX_BASE,
    .eepromAddress = (uint32_t)eepromData,
    .eepromSize = EEPROM_SIZE,
    .firmwareVersion = FC_VERSION_STRING,
};

static void faultHandler(uint32_t code)
{
    MBX_REG(MBX_FAULT) = code;
    while (1) {
        __asm__ volatile ("wfi");
    }
}

void NMI_Handler(void)        { faultHandler(2); }
void HardFault_Handler(void)  { faultHandler(3); }
void MemManage_Handler(void)  { faultHandler(4); }
void BusFault_Handler(void)   { faultHandler(5); }
void UsageFault_Handler(void) { faultHandler(6); }
void Default_Handler(void)    { faultHandler(0xff); }

void Reset_Handler(void)
{
    // Full access to the FPU (CPACR CP10 and CP11).
    *(volatile uint32_t *)0xE000ED88 |= (0xFu << 20);
    __asm__ volatile ("dsb\n isb");

    uint32_t *src = &_sidata;
    for (uint32_t *dst = &_sdata; dst < &_edata; ) {
        *dst++ = *src++;
    }
    for (uint32_t *dst = &_sbss; dst < &_ebss; ) {
        *dst++ = 0;
    }

    main(0, NULL);
    faultHandler(1);
}

typedef void (*vector_t)(void);

__attribute__((section(".isr_vector"), used))
const vector_t virtualVectorTable[16] = {
    (vector_t)(uintptr_t)&_estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    (vector_t)(uintptr_t)&virtualBoardInfo,   // reserved on ARMv7-M; the host finds the board here
    0, 0, 0,
    Default_Handler,               // SVC
    Default_Handler,               // DebugMon
    0,
    Default_Handler,               // PendSV
    Default_Handler,               // SysTick
};
