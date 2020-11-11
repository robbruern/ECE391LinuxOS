/* exception-handlers.c - Defines the exception handlers
 * vim:ts=4 noexpandtab
 */
#pragma once
#include "lib.h"
#include "terminal.h"
#include "syscalls.h"

extern void exception_div_zero();
extern void exception_debug();
extern void exception_nonmaskable_interrupt();
extern void exception_breakpoint();
extern void exception_overflow();
extern void exception_bound_range_exceeded();
extern void exception_invalid_opcode();
extern void exception_device_not_available();
extern void exception_double_fault();
extern void exception_coprocessor_segment_overrun();
extern void exception_invalid_TSS();
extern void exception_segment_not_present();
extern void exception_stack_segment_fault();
extern void exception_general_protection_fault();
extern void exception_page_fault();
extern void exception_reserved();
extern void exception_x87_floating_point();
extern void exception_alignment_check();
extern void exception_machine_check();
extern void exception_SIMD_floating_point();
extern void exception_virtualization();
extern void exception_security();
