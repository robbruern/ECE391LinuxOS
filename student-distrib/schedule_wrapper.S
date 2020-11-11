/* filename _wrapper.S */
.globl schedule_wrapper
.align 4

/*Function to be a wrapper around the handle_schedule function*/
schedule_wrapper:
    cli
    pushal
    pushfl
    movl %ebp, last_ebp
    movl %esp, last_esp
    call timer_handler
    movl last_ebp, %ebp
    movl  last_esp, %esp
    popfl
    popal
    iret
