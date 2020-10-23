/*
 * Copyright (c) 2012 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <stdint.h>

#include <lk/compiler.h>
#include <lk/debug.h>

#ifdef RISCV_VARIANT_NUCLEI
extern void _start(void);

/* un-overridden irq handler */
static void nuclei_core_dummy_irq(void) {
    panic("unhandled core irq\n");
}

void riscv_msip_handler(void) __WEAK_ALIAS("nuclei_core_dummy_irq");
void riscv_mtip_handler(void) __WEAK_ALIAS("nuclei_core_dummy_irq");

const void *const __SECTION(".text.boot.vectab1") vectab[] = {
    /* riscv clic core interrupts(0-18) */
    _start,         // 0, jump to _start
    0,              // 1, reserved
    0,              // 2, reserved
    riscv_msip_handler,   // 3, machine software interrupt
    0,              // 4, reserved
    0,              // 5, reserved
    0,              // 6, reserved
    riscv_mtip_handler,   // 7, machine software interrupt
    0,              // 8, reserved
    0,              // 9, reserved
    0,              // 10, reserved
    0,              // 11, reserved
    0,              // 12, reserved
    0,              // 13, reserved
    0,              // 14, reserved
    0,              // 15, reserved
    0,              // 16, reserved
    0,              // 17, reserved
    0,              // 18, reserved
};
#endif