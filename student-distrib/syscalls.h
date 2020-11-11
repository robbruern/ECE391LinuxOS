/* syscalls.h - System Call Handler
 * vim:ts=4 noexpandtab
 */
#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "types.h"
#include "lib.h"
#include "filesystem_driver.h"
#include "process_control.h"

#define TYPE_RTC 0
#define TYPE_DIR 1
#define TYPE_FILE 2

#define SYS_HALT        1
#define SYS_EXECUTE     2
#define SYS_READ        3
#define SYS_WRITE       4
#define SYS_OPEN        5
#define SYS_CLOSE       6
#define SYS_GETARGS     7
#define SYS_VIDMAP      8
#define SYS_SET_HANDLER 9
#define SYS_SIGRETURN   10

#define VIRTUAL_START 0x8048000
#define ENTRY_START 0x8048018
#define STACK_LOCATION 0x83FFFF0
#define MAGIC_LEAD 0x464c457f
#define VIRTUAL_PROGRAM 32
#define PAGE_ARGS 0x87

#define USER_VMEM 0x8400000
#define PROGMEM_UPPER 0x800000
#define PROGMEM_LOWER 0x400000
#define VIDMAP 0x8421000

#define VID_IDX 33

extern int do_call(int call, int arg0, int arg1, int arg2);
extern int handle_system_calls();
extern int32_t close(int32_t fd);
extern int32_t open(const uint8_t *filename);
extern int32_t write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t read(int32_t fd, void *buf, int32_t nbytes);
extern void pcb_close(int fd);

#define NUM_TERMINALS   3 // Should be in terminal.h
#define TOTAL_PROCESSES 7 // Should be in process_control.h

int shell_pid[NUM_TERMINALS];
int working_pid[NUM_TERMINALS];
int schedule_top[NUM_TERMINALS];                        // Holds the index of the top of each terminal's schedule stack
uint8_t schedule_stack[NUM_TERMINALS][TOTAL_PROCESSES]; // Holds the processes present within a terminal.
tss_t schedule_tss[NUM_TERMINALS][TOTAL_PROCESSES];     // Holds the tast state segment per process per terminal.
int schedule_eip[NUM_TERMINALS][TOTAL_PROCESSES];       // Holds the Instruction Pointer per process per terminal.
int schedule_esp[NUM_TERMINALS][TOTAL_PROCESSES];       // Holds the stack pointer per process per terminal.
int schedule_ebp[NUM_TERMINALS][TOTAL_PROCESSES];       // Holds the stack pointer per process per terminal.
#endif
