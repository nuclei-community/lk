/*
 * Copyright (c) 2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#ifdef RISCV_VARIANT_NUCLEI
#include <nuclei_sdk_soc.h>
#endif
#include <assert.h>
#include <lk/debug.h>
#include <lk/trace.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/thread.h>
#include <arch/riscv.h>

#define LOCAL_TRACE 1

volatile unsigned long  rt_interrupt_from_thread = 0;
volatile unsigned long  rt_interrupt_to_thread   = 0;
volatile unsigned long rt_thread_switch_interrupt_flag = 0;
#define portINITIAL_MSTATUS ( MSTATUS_MPP | MSTATUS_FS_INITIAL)

struct thread *_current_thread;

static void initial_thread_func(void) __NO_RETURN;
static void initial_thread_func(void) {
    DEBUG_ASSERT(arch_ints_disabled());

    thread_t *ct = get_current_thread();

#if LOCAL_TRACE
    LTRACEF("thread %p calling %p with arg %p\n", ct, ct->entry, ct->arg);
    dump_thread(ct);
#endif

    /* release the thread lock that was implicitly held across the reschedule */
    spin_unlock(&thread_lock);
    arch_enable_ints();
    
    LTRACEF("thread %s: %p, prio %d enter with %d\n", ct->name, ct, ct->priority, ct->arg);

    int ret = ct->entry(ct->arg);

    // LTRACEF("thread %p exiting with %d\n", ct, ret);
    LTRACEF("thread %s: %p exiting with %d\n", ct->name, ct, ret);

    thread_exit(ret);
}

void arch_thread_initialize(thread_t *t) {
    /* zero out the thread context */
    memset(&t->arch.cs_frame, 0, sizeof(t->arch.cs_frame));

    /* make sure the top of the stack is 16 byte aligned */
    vaddr_t stack_top = ROUNDDOWN((vaddr_t)t->stack + t->stack_size, 16);
#ifndef RISCV_VARIANT_NUCLEI
    t->arch.cs_frame.sp = stack_top;
    t->arch.cs_frame.ra = (vaddr_t)&initial_thread_func;
#else
    extern void arch_idle(void);
    stack_top -= sizeof(struct riscv_context_switch_frame);
    t->arch.cs_frame = (struct riscv_context_switch_frame *)stack_top;
    t->arch.cs_frame->ra = (unsigned long)&arch_idle;
    t->arch.cs_frame->a0 = (unsigned long)t->arg;
    t->arch.cs_frame->epc = (unsigned long)&initial_thread_func;
    t->arch.cs_frame->mstatus = (unsigned long)portINITIAL_MSTATUS;
#endif
    LTRACEF("t %p (%s) stack top %#lx entry %p arg %p\n", t, t->name, stack_top, t->entry, t->arg);
}

extern void arch_context_start(unsigned long sp);

void arch_context_switch(thread_t *oldthread, thread_t *newthread) {
    DEBUG_ASSERT(arch_ints_disabled());

    LTRACEF("old %p (%s), new %p (%s)\n", oldthread, oldthread->name, newthread, newthread->name);
    printf("int %d, old %p (%s), new %p (%s)\n", rt_thread_switch_interrupt_flag, oldthread, oldthread->name, newthread, newthread->name);
#ifdef RISCV_VARIANT_NUCLEI
    LTRACEF("old %s (sp %p), new %s (sp %p)\n", oldthread->name, oldthread->arch.cs_frame, newthread->name, newthread->arch.cs_frame);
    if (oldthread->stack_size > 0) {
        // if (rt_thread_switch_interrupt_flag == 0) {
            rt_interrupt_from_thread = &(oldthread->arch.cs_frame);
        // }
        rt_interrupt_to_thread = &(newthread->arch.cs_frame);
        riscv_trigger_preempt();
        // if (rt_thread_switch_interrupt_flag > 0) {
        //     riscv_trigger_preempt();
        // } else {
        //     //__enable_irq();
        //     riscv_context_switch(rt_interrupt_from_thread, rt_interrupt_to_thread);
            
        //     // __disable_irq();
        // }
        
    } else { // First task started
        arch_context_start((unsigned long)&(newthread->arch.cs_frame));
        // never return
    }
    
#else
    LTRACEF("old %s (sp %p), new %s (sp %p)\n", oldthread->name, &oldthread->arch.cs_frame, newthread->name, &newthread->arch.cs_frame);
    // LTRACEF("from pc %p, to pc %p\n", oldthread->arch.cs_frame.ra, newthread->arch.cs_frame.ra);
    rt_interrupt_from_thread = &(oldthread->arch.cs_frame);
    rt_interrupt_to_thread = &(newthread->arch.cs_frame);
    riscv_context_switch(&oldthread->arch.cs_frame, &newthread->arch.cs_frame);
#endif
}

void arch_dump_thread(thread_t *t) {
    if (t->state != THREAD_RUNNING) {
        dprintf(INFO, "\tarch: ");
#ifdef RISCV_VARIANT_NUCLEI
        dprintf(INFO, "sp %#lx\n", t->arch.cs_frame);
#else
        dprintf(INFO, "sp %#lx\n", t->arch.cs_frame.sp);
#endif
    }
}
#ifdef RISCV_VARIANT_NUCLEI
void xPortTaskSwitch( void )
{
    /* Clear Software IRQ, A MUST */
    SysTimer_ClearSWIRQ();
    struct riscv_context_switch_frame *from_frame = *((struct riscv_context_switch_frame **)rt_interrupt_from_thread);
    struct riscv_context_switch_frame *to_frame = *((struct riscv_context_switch_frame **)rt_interrupt_to_thread);
    LTRACEF("int %d, from pc %p, %p, to pc %p %p\n", rt_thread_switch_interrupt_flag, \
            from_frame->epc, from_frame->mstatus, to_frame->epc, to_frame->mstatus);
    rt_thread_switch_interrupt_flag = 0;
    printf("Perform task switch, mepc %p, clear to %d\n", __RV_CSR_READ(CSR_MEPC), rt_thread_switch_interrupt_flag);
}

void riscv_trigger_preempt(void)
{
    /* Set a software interrupt(SWI) request to request a context switch. */
    SysTimer_SetSWIRQ();
    __RWMB();
}
#else
void xPortTaskSwitch( void )
{
    printf("Perform task switch\n");
    struct riscv_context_switch_frame *from_frame = (struct riscv_context_switch_frame *)rt_interrupt_from_thread;
    struct riscv_context_switch_frame *to_frame = (struct riscv_context_switch_frame *)rt_interrupt_to_thread;
    LTRACEF("int %d, from pc %p, to pc %p\n", rt_thread_switch_interrupt_flag, from_frame->ra, to_frame->ra);
}
#endif