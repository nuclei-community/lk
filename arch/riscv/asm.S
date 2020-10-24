/*
 * Copyright (c) 2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */
#include <lk/asm.h>
#include <arch/riscv.h>
#include <arch/riscv/asm.h>
/* void riscv_context_switch(
    struct riscv_context_switch_frame *oldcs,
    struct riscv_context_switch_frame *newcs); */
FUNCTION(riscv_context_switch)
    # a0 = oldcs
    # a1 = newcs
    STR    ra, REGOFF(0)(a0)
    STR    sp, REGOFF(1)(a0)
    STR    s0, REGOFF(2)(a0)
    STR    s1, REGOFF(3)(a0)
    STR    s2, REGOFF(4)(a0)
    STR    s3, REGOFF(5)(a0)
    STR    s4, REGOFF(6)(a0)
    STR    s5, REGOFF(7)(a0)
    STR    s6, REGOFF(8)(a0)
    STR    s7, REGOFF(9)(a0)
    STR    s8, REGOFF(10)(a0)
    STR    s9, REGOFF(11)(a0)
    STR    s10, REGOFF(12)(a0)
    STR    s11, REGOFF(13)(a0)

    /* dump all the callee trashed regs on the stack */
    addi   sp, sp, -REGOFF(20) // subtract a multiple of 16 to align the stack in 32bit
    STR    t6, REGOFF(17)(sp)
    STR    t5, REGOFF(16)(sp)
    STR    t4, REGOFF(15)(sp)
    STR    t3, REGOFF(14)(sp)
    STR    t2, REGOFF(13)(sp)
    STR    t1, REGOFF(12)(sp)
    STR    t0, REGOFF(11)(sp)
    STR    a7, REGOFF(10)(sp)
    STR    a6, REGOFF(9)(sp)
    STR    a5, REGOFF(8)(sp)
    STR    a4, REGOFF(7)(sp)
    STR    a3, REGOFF(6)(sp)
    STR    a2, REGOFF(5)(sp)
    STR    a1, REGOFF(4)(sp)
    STR    a0, REGOFF(3)(sp)
    STR    ra, REGOFF(2)(sp)

    jal xPortTaskSwitch

    /* put everything back */

    LDR    ra, REGOFF(2)(sp)
    LDR    a0, REGOFF(3)(sp)
    LDR    a1, REGOFF(4)(sp)
    LDR    a2, REGOFF(5)(sp)
    LDR    a3, REGOFF(6)(sp)
    LDR    a4, REGOFF(7)(sp)
    LDR    a5, REGOFF(8)(sp)
    LDR    a6, REGOFF(9)(sp)
    LDR    a7, REGOFF(10)(sp)
    LDR    t0, REGOFF(11)(sp)
    LDR    t1, REGOFF(12)(sp)
    LDR    t2, REGOFF(13)(sp)
    LDR    t3, REGOFF(14)(sp)
    LDR    t4, REGOFF(15)(sp)
    LDR    t5, REGOFF(16)(sp)
    LDR    t6, REGOFF(17)(sp)
    addi   sp, sp, REGOFF(20)

    LDR    s11, REGOFF(13)(a1)
    LDR    s10, REGOFF(12)(a1)
    LDR    s9, REGOFF(11)(a1)
    LDR    s8, REGOFF(10)(a1)
    LDR    s7, REGOFF(9)(a1)
    LDR    s6, REGOFF(8)(a1)
    LDR    s5, REGOFF(7)(a1)
    LDR    s4, REGOFF(6)(a1)
    LDR    s3, REGOFF(5)(a1)
    LDR    s2, REGOFF(4)(a1)
    LDR    s1, REGOFF(3)(a1)
    LDR    s0, REGOFF(2)(a1)
    LDR    sp, REGOFF(1)(a1)
    LDR    ra, REGOFF(0)(a1)

    ret
END_FUNCTION(riscv_context_switch)

#ifndef RISCV_VARIANT_NUCLEI
/* top level exception handler for riscv in non vectored mode */
.balign 4
FUNCTION(riscv_exception_entry)
    /* dump all the callee trashed regs on the stack */
    addi   sp, sp, -REGOFF(20) // subtract a multiple of 16 to align the stack in 32bit
    STR    t6, REGOFF(17)(sp)
    STR    t5, REGOFF(16)(sp)
    STR    t4, REGOFF(15)(sp)
    STR    t3, REGOFF(14)(sp)
    STR    t2, REGOFF(13)(sp)
    STR    t1, REGOFF(12)(sp)
    STR    t0, REGOFF(11)(sp)
    STR    a7, REGOFF(10)(sp)
    STR    a6, REGOFF(9)(sp)
    STR    a5, REGOFF(8)(sp)
    STR    a4, REGOFF(7)(sp)
    STR    a3, REGOFF(6)(sp)
    STR    a2, REGOFF(5)(sp)
    STR    a1, REGOFF(4)(sp)
    STR    a0, REGOFF(3)(sp)
    STR    ra, REGOFF(2)(sp)
    csrr   t0, RISCV_CSR_XSTATUS
    STR    t0, REGOFF(1)(sp)
    csrr   a0, RISCV_CSR_XCAUSE
    csrr   a1, RISCV_CSR_XEPC
    STR    a1, REGOFF(0)(sp)
    mv     a2, sp

    jal    riscv_exception_handler

    /* put everything back */
    LDR    t0, REGOFF(0)(sp)
    csrw   RISCV_CSR_XEPC, t0
    LDR    t0, REGOFF(1)(sp)
    csrw   RISCV_CSR_XSTATUS, t0

    LDR    ra, REGOFF(2)(sp)
    LDR    a0, REGOFF(3)(sp)
    LDR    a1, REGOFF(4)(sp)
    LDR    a2, REGOFF(5)(sp)
    LDR    a3, REGOFF(6)(sp)
    LDR    a4, REGOFF(7)(sp)
    LDR    a5, REGOFF(8)(sp)
    LDR    a6, REGOFF(9)(sp)
    LDR    a7, REGOFF(10)(sp)
    LDR    t0, REGOFF(11)(sp)
    LDR    t1, REGOFF(12)(sp)
    LDR    t2, REGOFF(13)(sp)
    LDR    t3, REGOFF(14)(sp)
    LDR    t4, REGOFF(15)(sp)
    LDR    t5, REGOFF(16)(sp)
    LDR    t6, REGOFF(17)(sp)
    addi   sp, sp, REGOFF(20)

    RISCV_XRET
END_FUNCTION(riscv_exception_entry)
#endif

#ifdef RISCV_VARIANT_NUCLEI
#include <riscv_encoding.h>

#ifndef __riscv_32e
#define portRegNum          30
#else
#define portRegNum          14
#endif

#define portCONTEXT_SIZE    ( portRegNum * REGBYTES )

    .extern rt_interrupt_from_thread
    .extern rt_interrupt_to_thread

.align 8

/**
 * \brief  Global interrupt disabled
 * \details
 *  This function disable global interrupt.
 * \remarks
 *  - All the interrupt requests will be ignored by CPU.
 */
.macro DISABLE_MIE
    csrc CSR_MSTATUS, MSTATUS_MIE
.endm

/**
 * \brief  Macro for context save
 * \details
 * This macro save ABI defined caller saved registers in the stack.
 * \remarks
 * - This Macro could use to save context when you enter to interrupt
 * or exception
*/
/* Save caller registers */
.macro SAVE_CONTEXT
    csrrw sp, CSR_MSCRATCHCSWL, sp
    /* Allocate stack space for context saving */
#ifndef __riscv_32e
    addi sp, sp, -20*REGBYTES
#else
    addi sp, sp, -14*REGBYTES
#endif /* __riscv_32e */

    STORE x1, 0*REGBYTES(sp)
    /* STORE x4, 1*REGBYTES(sp) */
    STORE x5, 2*REGBYTES(sp)
    STORE x6, 3*REGBYTES(sp)
    STORE x7, 4*REGBYTES(sp)
    STORE x10, 5*REGBYTES(sp)
    STORE x11, 6*REGBYTES(sp)
    STORE x12, 7*REGBYTES(sp)
    STORE x13, 8*REGBYTES(sp)
    STORE x14, 9*REGBYTES(sp)
    STORE x15, 10*REGBYTES(sp)
#ifndef __riscv_32e
    STORE x16, 14*REGBYTES(sp)
    STORE x17, 15*REGBYTES(sp)
    STORE x28, 16*REGBYTES(sp)
    STORE x29, 17*REGBYTES(sp)
    STORE x30, 18*REGBYTES(sp)
    STORE x31, 19*REGBYTES(sp)
#endif /* __riscv_32e */
.endm

/* Save caller registers on task stack */
.macro SAVE_CONTEXT_ON_TASK
    /* Allocate stack space for context saving */
#ifndef __riscv_32e
    addi sp, sp, -20*REGBYTES
#else
    addi sp, sp, -14*REGBYTES
#endif /* __riscv_32e */

    STORE x1, 0*REGBYTES(sp)
    /* STORE x4, 1*REGBYTES(sp) */
    STORE x5, 2*REGBYTES(sp)
    STORE x6, 3*REGBYTES(sp)
    STORE x7, 4*REGBYTES(sp)
    STORE x10, 5*REGBYTES(sp)
    STORE x11, 6*REGBYTES(sp)
    STORE x12, 7*REGBYTES(sp)
    STORE x13, 8*REGBYTES(sp)
    STORE x14, 9*REGBYTES(sp)
    STORE x15, 10*REGBYTES(sp)
#ifndef __riscv_32e
    STORE x16, 14*REGBYTES(sp)
    STORE x17, 15*REGBYTES(sp)
    STORE x28, 16*REGBYTES(sp)
    STORE x29, 17*REGBYTES(sp)
    STORE x30, 18*REGBYTES(sp)
    STORE x31, 19*REGBYTES(sp)
#endif /* __riscv_32e */
.endm

/**
 * \brief  Macro for restore caller registers
 * \details
 * This macro restore ABI defined caller saved registers from stack.
 * \remarks
 * - You could use this macro to restore context before you want return
 * from interrupt or exeception
 */
/* Restore caller registers */
.macro RESTORE_CONTEXT
    LOAD x1, 0*REGBYTES(sp)
    /* LOAD x4, 1*REGBYTES(sp) */
    LOAD x5, 2*REGBYTES(sp)
    LOAD x6, 3*REGBYTES(sp)
    LOAD x7, 4*REGBYTES(sp)
    LOAD x10, 5*REGBYTES(sp)
    LOAD x11, 6*REGBYTES(sp)
    LOAD x12, 7*REGBYTES(sp)
    LOAD x13, 8*REGBYTES(sp)
    LOAD x14, 9*REGBYTES(sp)
    LOAD x15, 10*REGBYTES(sp)
#ifndef __riscv_32e
    LOAD x16, 14*REGBYTES(sp)
    LOAD x17, 15*REGBYTES(sp)
    LOAD x28, 16*REGBYTES(sp)
    LOAD x29, 17*REGBYTES(sp)
    LOAD x30, 18*REGBYTES(sp)
    LOAD x31, 19*REGBYTES(sp)

    /* De-allocate the stack space */
    addi sp, sp, 20*REGBYTES
#else
    /* De-allocate the stack space */
    addi sp, sp, 14*REGBYTES
#endif /* __riscv_32e */
    csrrw sp, CSR_MSCRATCHCSWL, sp
.endm

/* Restore caller registers on task */
.macro RESTORE_CONTEXT_ON_TASK
    LOAD x1, 0*REGBYTES(sp)
    /* LOAD x4, 1*REGBYTES(sp) */
    LOAD x5, 2*REGBYTES(sp)
    LOAD x6, 3*REGBYTES(sp)
    LOAD x7, 4*REGBYTES(sp)
    LOAD x10, 5*REGBYTES(sp)
    LOAD x11, 6*REGBYTES(sp)
    LOAD x12, 7*REGBYTES(sp)
    LOAD x13, 8*REGBYTES(sp)
    LOAD x14, 9*REGBYTES(sp)
    LOAD x15, 10*REGBYTES(sp)
#ifndef __riscv_32e
    LOAD x16, 14*REGBYTES(sp)
    LOAD x17, 15*REGBYTES(sp)
    LOAD x28, 16*REGBYTES(sp)
    LOAD x29, 17*REGBYTES(sp)
    LOAD x30, 18*REGBYTES(sp)
    LOAD x31, 19*REGBYTES(sp)

    /* De-allocate the stack space */
    addi sp, sp, 20*REGBYTES
#else
    /* De-allocate the stack space */
    addi sp, sp, 14*REGBYTES
#endif /* __riscv_32e */
.endm

/**
 * \brief  Macro for save necessary CSRs to stack
 * \details
 * This macro store MCAUSE, MEPC, MSUBM to stack.
 */
.macro SAVE_CSR_CONTEXT
    /* Store CSR mcause to stack using pushmcause */
    csrrwi  x0, CSR_PUSHMCAUSE, 11
    /* Store CSR mepc to stack using pushmepc */
    csrrwi  x0, CSR_PUSHMEPC, 12
    /* Store CSR msub to stack using pushmsub */
    csrrwi  x0, CSR_PUSHMSUBM, 13
.endm

/**
 * \brief  Macro for restore necessary CSRs from stack
 * \details
 * This macro restore MSUBM, MEPC, MCAUSE from stack.
 */
.macro RESTORE_CSR_CONTEXT
    LOAD x5,  13*REGBYTES(sp)
    csrw CSR_MSUBM, x5
    LOAD x5,  12*REGBYTES(sp)
    csrw CSR_MEPC, x5
    LOAD x5,  11*REGBYTES(sp)
    csrw CSR_MCAUSE, x5
.endm

/**
 * \brief  Exception/NMI Entry
 * \details
 * This function provide common entry functions for exception/nmi.
 * \remarks
 * This function provide a default exception/nmi entry.
 * ABI defined caller save register and some CSR registers
 * to be saved before enter interrupt handler and be restored before return.
 */
.section .text.trap
/* In CLIC mode, the exeception entry must be 64bytes aligned */
.align 6
.global exc_entry
exc_entry:
    /* Save the caller saving registers (context) */
    SAVE_CONTEXT
    /* Save the necessary CSR registers */
    SAVE_CSR_CONTEXT

    /*
     * Set the exception handler function arguments
     * argument 1: mcause value
     * argument 2: current stack point(SP) value
     */
    csrr a0, mcause
    mv a1, sp
    /*
     * TODO: Call the exception handler function
     * By default, the function template is provided in
     * system_Device.c, you can adjust it as you want
     */
    call core_exception_handler

    /* Restore the necessary CSR registers */
    RESTORE_CSR_CONTEXT
    /* Restore the caller saving registers (context) */
    RESTORE_CONTEXT

    /* Return to regular code */
    mret

/**
 * \brief  Non-Vector Interrupt Entry
 * \details
 * This function provide common entry functions for handling
 * non-vector interrupts
 * \remarks
 * This function provide a default non-vector interrupt entry.
 * ABI defined caller save register and some CSR registers need
 * to be saved before enter interrupt handler and be restored before return.
 */
.section      .text.irq
/* In CLIC mode, the interrupt entry must be 4bytes aligned */
.align 2
.global irq_entry
/* This label will be set to MTVT2 register */
irq_entry:
    /* Save the caller saving registers (context) */
    SAVE_CONTEXT
    /* Save the necessary CSR registers */
    SAVE_CSR_CONTEXT

    /* This special CSR read/write operation, which is actually
     * claim the CLIC to find its pending highest ID, if the ID
     * is not 0, then automatically enable the mstatus.MIE, and
     * jump to its vector-entry-label, and update the link register
     */
    csrrw ra, CSR_JALMNXTI, ra

    /* Critical section with interrupts disabled */
    DISABLE_MIE

    jal riscv_irq_exit

    /* Restore the necessary CSR registers */
    RESTORE_CSR_CONTEXT
    /* Restore the caller saving registers (context) */
    RESTORE_CONTEXT

    /* Return to regular code */
    mret


.align 2
.global riscv_msip_handler
riscv_msip_handler:
    /* Save the caller saving registers (context) */
    SAVE_CONTEXT
    /* Save the necessary CSR registers */
    SAVE_CSR_CONTEXT

    // Jump to thread preemt
    jal riscv_do_preempt

    /* Restore the necessary CSR registers */
    RESTORE_CSR_CONTEXT
    /* Restore the caller saving registers (context) */
    RESTORE_CONTEXT

    /* Return to regular code */
    mret

#endif