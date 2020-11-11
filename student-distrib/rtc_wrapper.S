/* filename rtc_wrapper.S */
.globl rtc_wrapper
.align 4

/*Function to be a wrapper around the handle_RTC function*/
rtc_wrapper:
    pushal  /*Save registers*/
    cld
    call handle_RTC
    popal   /*Restore registers*/
    iret    /*Return from interrupt*/
