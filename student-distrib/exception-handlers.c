/* exception-handlers.c - Defines the exception handlers
 * vim:ts=4 noexpandtab
 */
#include "exception-handlers.h"

/*
 * exception_div_zero
 *   DESCRIPTION: Exception handler which is called when there is a divide
 *   by zero. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_div_zero()
{
    printf("Exception: Divide by Zero\n");
    int status = 256;
    do_call(SYS_HALT, status, 0, 0);
}

/*
 * exception_debug
 *   DESCRIPTION: Exception handler which is called when there is a debug
 *   exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_debug(){
    printf("Exception: Debug\n");
    while(1){}
}

/*
 * exception_nonmaskable_interrupt
 *   DESCRIPTION: Exception handler which is called when there is a nonmaskable
 *   intterupt exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_nonmaskable_interrupt(){
    printf("Exception: Non-maskable Interrupt\n");
    while(1){}
}

/*
 * exception_breakpoints
 *   DESCRIPTION: Exception handler which is called when there is a breakpoint
 *   exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_breakpoint(){
    printf("Exception: Breakpoint\n");
    while(1){}
}

/*
 * exception_overflow
 *   DESCRIPTION: Exception handler which is called when there is an overflow
 *   exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_overflow(){
    printf("Exception: overflow\n");
    int status = 256;
    do_call(SYS_HALT, status, 0, 0);
}

/*
 * exception_bound_range_exceeded
 *   DESCRIPTION: Exception handler which is called when there is a bound range
 *   exceeded exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_bound_range_exceeded(){
    printf("Exception: Breakpoint\n");
    // while(1){}
    int status = 256;
    do_call(SYS_HALT, status, 0, 0);

}

/*
 * exception_invalid_opcode
 *   DESCRIPTION: Exception handler which is called when there is an invalid
 *   opcode exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_invalid_opcode(){
    printf("Exception: Invalid opcode\n");
    // while(1){}
    int status = 256;
    do_call(SYS_HALT, status, 0, 0);

}

/*
 * exception_device_not_available
 *   DESCRIPTION: Exception handler which is called when there is a device not
 *   available exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_device_not_available(){
    printf("Exception: Device Not Available\n");
    while(1){}
}

/*
 * exception_double_fault
 *   DESCRIPTION: Exception handler which is called when there is a double fault
 *   exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_double_fault(){
    printf("Exception: Double Fault\n");
    while(1){}
}

/*
 * exception_coprocessor_segment_overrun
 *   DESCRIPTION: Exception handler which is called when there is a coprocessor
 *   segement overrun exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_coprocessor_segment_overrun(){
    printf("Exception: CoProcessor Segment Overun\n");
    while(1){}
}

/*
 * exception_invalid_TSS
 *   DESCRIPTION: Exception handler which is called when there is an invalid
 *   TSS exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_invalid_TSS(){
    printf("Exception: Invalid TSS\n");
    while(1){}
}

/*
 * exception_segment_not_present
 *   DESCRIPTION: Exception handler which is called when there is a segment not
 *   present exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_segment_not_present(){
    printf("Exception: Segment Not Present\n");
    while(1){}
}

/*
 * exception_page_fault
 *   DESCRIPTION: Exception handler which is called when there is a page fault
 *   exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_page_fault(){
    printf("Exception: Page Fault\n");
    // while(1){}
    int status = 256;
    do_call(SYS_HALT, status, 0, 0);
}

/*
 * exception_reserved
 *   DESCRIPTION: Exception handler which is called when there is a bound range
 *   exceeded exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_reserved(){
    printf("Exception: Reserved. How did you get here?\n");
    while(1){}
}

/*
 * exception_x87_floating_point
 *   DESCRIPTION: Exception handler which is called when there is a x87 floating
 *   point exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_x87_floating_point(){
    printf("Exception: x87 Floating-Point Exception\n");
    while(1){}
}

/*
 * exception_alignment_check
 *   DESCRIPTION: Exception handler which is called when there is an alignment
 *   check exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_alignment_check(){
    printf("Exception: Alignment Check\n");
    while(1){}
}

/*
 * exception_machine_check
 *   DESCRIPTION: Exception handler which is called when there is a machine
 *   check exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_machine_check(){
    printf("Exception: Machine Check\n");
    while(1){}
}

/*
 * exception_SIMD_floating_point
 *   DESCRIPTION: Exception handler which is called when there is a SIMD
 *   floating point exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_SIMD_floating_point(){
    printf("Exception: SIMD Floating-Point Exception\n");
    while(1){}
}

/*
 * exception_virtualization
 *   DESCRIPTION: Exception handler which is called when there is a
 *   virtualization exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_virtualization(){
    printf("Exception: Virtualization Exception\n");
    while(1){}
}

/*
 * exception_security
 *   DESCRIPTION: Exception handler which is called when there is a security
 *   exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_security(){
    printf("Exception: Security Exception\n");
    while(1){}
}

/*
 * exception_general_protection_fault
 *   DESCRIPTION: Exception handler which is called when there is a general
 *   protection fault exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_general_protection_fault() {
    // int bad_eip = 0;

    // __asm__("movl 4(%%esp), %0"
    //         : "=r"(bad_eip)
    //         );

    printf("Exception: General Protection Fault\n");
    // while(1){}
    int status = 256;
    do_call(SYS_HALT, status, 0, 0);
}

/*
 * exception_stack_segment_fault
 *   DESCRIPTION: Exception handler which is called when there is a stack
 *   segment fault exception. Will print the exception and wait forever.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Waits forever
 */
void exception_stack_segment_fault(){
    printf("Exception: Stack Segment Fault\n");
    // while(1){}
    int status = 256;
    do_call(SYS_HALT, status, 0, 0);
}
