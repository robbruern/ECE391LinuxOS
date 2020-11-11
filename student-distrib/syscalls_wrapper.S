/* filename syscalls_wrapper.S */
.globl system_call
.align 4

return_val:                 .long 0x0
/*Function to be a wrapper around the syscall functions*/
system_call:
				cli
        cmpl $1, %eax
        jl SYSCALL_ERROR
        cmpl $10, %eax
        ja SYSCALL_ERROR
        decl %eax
        pushal

        pushl %edx
        pushl %ecx
        pushl %ebx
        call *syscalltable(, %eax, 4)
        movl %eax, return_val
        add $12, %esp

        popal
        movl return_val, %eax
        jmp SYSCALL_RETURN

    SYSCALL_ERROR:
        mov $-1, %eax
        jmp SYSCALL_RETURN

    SYSCALL_RETURN:
        iret

syscalltable:
    .long halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
