/*
 * Copyright (c) 2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <assert.h>
#include <lk/trace.h>
#include <lk/debug.h>
#include <stdint.h>
#include <arch/riscv.h>
#include <arch/ops.h>
#include <arch/mp.h>
#include <lk/init.h>
#include <lk/main.h>
#include <platform.h>

#include "riscv_priv.h"

#ifdef RISCV_VARIANT_NUCLEI
#include <riscv_encoding.h>
#endif

#define LOCAL_TRACE 0

// per cpu structure, pointed to by xscratch
struct riscv_percpu percpu[RISCV_MAX_HARTS];
volatile unsigned long riscv_reschedule = 0;

// called extremely early from start.S prior to getting into any other C code on
// both the boot cpu and the secondaries
void riscv_configure_percpu_early(uint hart_id) {
    // point xscratch at the current cpu
    // on the first cpu cpu_num should be set to 0 so we'll leave it alone
    // on secondary cpus the secondary boot code will fill in the cpu number
    riscv_csr_write(RISCV_CSR_XSCRATCH, &percpu[hart_id]);
    percpu[hart_id].hart_id = hart_id;
}

// first C level code to initialize each cpu
void riscv_early_init_percpu(void) {
#ifdef RISCV_VARIANT_NUCLEI
    extern void *vectab;
    extern void exc_entry(void);
    extern void irq_entry(void);
    extern void _premain_init(void);
    extern void platform_init_timer(void);
    extern unsigned long default_stack_top;
    unsigned long entry_tmp = 0;

    // set nmi exception to mtvec
    riscv_csr_set(CSR_MMISC_CTL, MMISC_CTL_NMI_CAUSE_FFF);
    // set clic vector base
    riscv_csr_write(CSR_MTVT, (uintptr_t)&vectab);
    entry_tmp = ((unsigned long)irq_entry) | 0x1;
    // set clic non-vector irq entry
    riscv_csr_write(CSR_MTVT2, (uintptr_t)entry_tmp);
    entry_tmp = (((unsigned long)exc_entry) & (~(0x3FUL))) | 0x3;
    // set exception entry and enable clic mode
    riscv_csr_write(CSR_MTVEC, (uintptr_t)entry_tmp);
    // enable cycle and instret counter
    riscv_csr_set(mcounteren, 0x5);
    // set csr mscratch for interrupt stack usage
    riscv_csr_write(CSR_MSCRATCH, &default_stack_top);
    
    _premain_init();

    platform_init_timer();
    // mask all exceptions, just in case
    riscv_csr_clear(RISCV_CSR_XSTATUS, RISCV_CSR_XSTATUS_IE);
#else
    // set the top level exception handler
    riscv_csr_write(RISCV_CSR_XTVEC, (uintptr_t)&riscv_exception_entry);
    // mask all exceptions, just in case
    riscv_csr_clear(RISCV_CSR_XSTATUS, RISCV_CSR_XSTATUS_IE);
    riscv_csr_clear(RISCV_CSR_XIE, RISCV_CSR_XIE_SIE | RISCV_CSR_XIE_TIE | RISCV_CSR_XIE_EIE);

    // enable cycle counter (disabled for now, unimplemented on sifive-e)
    //riscv_csr_set(mcounteren, 1);
#endif
}

// called very early just after entering C code on boot processor
void arch_early_init(void) {
    riscv_early_init_percpu();
}

// later init per cpu
void riscv_init_percpu(void) {
#if WITH_SMP
    // enable software interrupts, used for inter-processor-interrupts
    riscv_csr_set(RISCV_CSR_XIE, RISCV_CSR_XIE_SIE);
#endif

    // enable external interrupts
    riscv_csr_set(RISCV_CSR_XIE, RISCV_CSR_XIE_EIE);
}

void riscv_clic_irq_entry(void) {
    THREAD_STATS_INC(interrupts);
}

void riscv_clic_irq_exit(bool reschedule) {
    if (reschedule != INT_NO_RESCHEDULE) {
        riscv_reschedule = reschedule;
    }
}


// called later once the kernel is running before platform and target init
void arch_init(void) {
    riscv_init_percpu();

    // print some arch info
#if RISCV_M_MODE
    dprintf(INFO, "RISCV: Machine mode\n");
    dprintf(INFO, "RISCV: mvendorid %#lx marchid %#lx mimpid %#lx mhartid %#x\n",
            riscv_get_mvendorid(), riscv_get_marchid(),
            riscv_get_mimpid(), riscv_current_hart());
    dprintf(INFO, "RISCV: misa %#lx\n", riscv_csr_read(RISCV_CSR_MISA));
#else
    dprintf(INFO, "RISCV: Supervisor mode\n");
#if RISCV_MMU
    dprintf(INFO, "RISCV: MMU enabled sv%u\n", RISCV_MMU);
#endif
    dprintf(INFO, "RISCV: SBI impl id %#lx version %#lx\n", sbi_call(SBI_GET_SBI_IMPL_ID).value, sbi_call(SBI_GET_SBI_IMPL_VERSION).value);

    // probe some SBI extensions
    dprintf(INFO, "RISCV: SBI extension TIMER %ld\n", sbi_call(SBI_PROBE_EXTENSION, SBI_EXT_TIMER).value);
    dprintf(INFO, "RISCV: SBI extension IPI %ld\n", sbi_call(SBI_PROBE_EXTENSION, SBI_EXT_IPI).value);
    dprintf(INFO, "RISCV: SBI extension RFENCE %ld\n", sbi_call(SBI_PROBE_EXTENSION, SBI_EXT_RFENCE).value);
    dprintf(INFO, "RISCV: SBI extension HSM %ld\n", sbi_call(SBI_PROBE_EXTENSION, SBI_EXT_HSM).value);
#endif

#if WITH_SMP
    riscv_boot_secondaries();
#endif
}

void arch_idle(void) {
    // let the platform/target disable wfi
#if !RISCV_DISABLE_WFI
    // __asm__ volatile("wfi");
#endif
}

void arch_chain_load(void *entry, ulong arg0, ulong arg1, ulong arg2, ulong arg3) {
    PANIC_UNIMPLEMENTED;
}

/* unimplemented cache operations */
#if RISCV_NO_CACHE_OPS
void arch_disable_cache(uint flags) { }
void arch_enable_cache(uint flags) { }

void arch_clean_cache_range(addr_t start, size_t len) { }
void arch_clean_invalidate_cache_range(addr_t start, size_t len) { }
void arch_invalidate_cache_range(addr_t start, size_t len) { }
void arch_sync_cache_range(addr_t start, size_t len) { }
#else
void arch_disable_cache(uint flags) { PANIC_UNIMPLEMENTED; }
void arch_enable_cache(uint flags) { PANIC_UNIMPLEMENTED; }

void arch_clean_cache_range(addr_t start, size_t len) { PANIC_UNIMPLEMENTED; }
void arch_clean_invalidate_cache_range(addr_t start, size_t len) { PANIC_UNIMPLEMENTED; }
void arch_invalidate_cache_range(addr_t start, size_t len) { PANIC_UNIMPLEMENTED; }
void arch_sync_cache_range(addr_t start, size_t len) { PANIC_UNIMPLEMENTED; }
#endif
