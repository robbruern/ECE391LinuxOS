/* filename keyboard_wrapper.S */
.globl keyboard_wrapper
.align 4

/*Function to be a wrapper around the handle_Keyboard function*/
keyboard_wrapper:
    pushal
    cld
    call handle_keyboard
    popal
    iret
