/* rtc_driver.c - Real-time clock driver.
 * vim:ts=4 noexpandtab
 */
#include "lib.h"
#include "interrupts.h"
#include "keyboard.h"
#include "x86_desc.h"
#include "filesystem_driver.h"
#include "rtc_driver.h"
static optable_t * rtc;
static optable_t  rtc_table;


void rtc_optable()
{
  rtc = &rtc_table;
  rtc->open = &rtc_open;
  rtc->read = &rtc_read;
  rtc->write = &rtc_write;
  rtc->close = &rtc_close;
}

/*
 * rtc_open
 *   DESCRIPTION: Sets the RTC frequency to twice a second
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: PCB index on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t rtc_open(const uint8_t* filename)
{
    int ret;            /** return value */
    int i;              /** loop variable */
    int32_t open;           /** keeps track of open spot on pcb */
    fd_block_t  fblock; /** file descriptor block to be assigned */
    int flags = 0;
    int rate = 15;
    ret = 0;
    i = 0;
    open = 0;
    cli_and_save(flags);


    /** returns -1 if file already open */
    for (i = 2; i < FDT_SIZE; i++)
    {
        if (control_blocks[current_pid].fd_table[i].flags == -1) {
          continue;
        }
        if (control_blocks[current_pid].fd_table[i].file_operations_pointer == rtc) {
            return -1;
        }
    }

    /** Check the ret */
    if (ret == -1) {
        return ret;
    }
    for (i = 2; i < FDT_SIZE; i++)
    {
        /** the control block is empty, take it */
        if (control_blocks[current_pid].fd_table[i].flags == -1) {
            open = i;
            break;
        }
        /** If there is not anavailable spot, return -1 */
        if (i == FDT_SIZE ) {
            return -1;
        }
    }

    // Need to disable interrupts

    outb(0x8A, CMOS_REG);
    char exisiting = inb(RTC_REG);
    outb(0x8A,CMOS_REG);

    /** Send the rate to the RTC */
    outb((exisiting & 0xF0) | rate, RTC_REG);

    // Need to enable interrupts
    restore_flags(flags);
    fblock.file_operations_pointer = rtc;
    fblock.inode = 0;
    fblock.flags = 1;
    // Set flags and file position?
    control_blocks[current_pid].fd_table[open] = fblock;
    sti();
    return open;
}

/*
 * rtc_close
 *   DESCRIPTION: returns zero
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t rtc_close(int32_t fd)
{
    cli();
    /** Check for fd validity */
    if (fd < 2 || fd >= FDT_SIZE) {
        return -1;
    }
    if (control_blocks[current_pid].fd_table[fd].flags == 0) {
        return -1;
    }
    /** Clear the control block entry */
    control_blocks[current_pid].fd_table[fd].file_operations_pointer = NULL;
    control_blocks[current_pid].fd_table[fd].flags = -1;
    control_blocks[current_pid].fd_table[fd].inode = -1;
    sti();
    return 0;
}

/*
 * rtc_read
 *   DESCRIPTION: Waits for an interrupt from the rtc and then writes character
 *   to the terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success
 *   SIDE EFFECTS: none
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes)
{
int cip = current_process;
	sti();
  rtc_virtual_interupt[cip] = 0;
    /* Wait for the interrupt to trigger the flag */
    while (rtc_virtual_interupt[cip] < 1) {}
  rtc_virtual_interupt[cip] = 0;
  cli();
    return SUCCESS;
}

/*
 * rtc_write
 *   DESCRIPTION: Sets the frequency of the RTC to the passed in rate
 *   INPUTS: int32_t rate
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t rate_size)
{
  int flags = 0;
  cli_and_save(flags);
    int rate = 0x0F;
    int passed_rate;
    int mask;
    memcpy(&passed_rate, buf, rate_size);
    for (mask = 2; mask <= MAX_RTC_RATE; mask *= 2)
    {
        if (mask == passed_rate) {
            break;
        } else if (mask == MAX_RTC_RATE) {
            return FAILURE;
        }
        rate--;
    }

    // Need to disable interrupts
    rtc_running[current_process] = passed_rate;
    rtc_virtual_interupt[current_process] = 0;
    cli_and_save(flags);
    outb(0x8A, CMOS_REG);
    char exisiting = inb(RTC_REG);
    outb(0x8A,CMOS_REG);
    /** Send the rate to the RTC */
    outb((exisiting & 0xF0) | rate, RTC_REG);

    // Need to enable interrupts
    restore_flags(flags);
    return 0;
}
