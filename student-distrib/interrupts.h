/* interrupts.h - Defines used idt
 * controller
 * vim:ts=4 noexpandtab
 */
#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#pragma once

#include "types.h"
#include "syscalls.h"
#include "filesystem_driver.h"

#define DPL_KERNEL 	0
#define DPL_USER 	3
#define PAGE_SIZE 	1024
#define VIDEOMEM 	184
#define VIDEO  0xB8000

#define VIDEO_1  0xB9000

#define KERNEL 		0x00400083

#define KERNEL_USER 0x00400087
#define SHELL 		0x00800087

#define STACK 		0x00C00087

#define PRESENT_MASK    0x00000001
#define READWRITE_MASK  0x00000002
#define USER_MASK		0x00000004

extern void handle_RTC();

extern int system_call_write(int32_t fd, const void * buf, int32_t nbytes);
extern void initialize_IDT();

extern void PIT_init();
extern void RTC_init();
extern void paging_init();
extern void keyboard_init();

extern  int rtc_interrupt_flag[3];
int rtc_running[3];
int rtc_virtual_interupt[3];

// extern int process_video_mem[3];

#define DPL_KERNEL  0
#define DPL_USER    3

#define DAC_DATA_REGISTER       0x3C9
#define DAC_ADDRESS_WRITE       0x3C8

#define CRTC_ADDRESS_REGISTER   0x3D4
#define CRTC_DATA_REGISTER      0x3D5

#define COMMAND_REGISTER      0x43
#define CHANNEL_0_DATA_REGISTER 0x40

#define IRQ0    0x00
#define IRQ1    0x01
#define IRQ2    0x02
#define IRQ3    0x03
#define IRQ4    0x04
#define IRQ5    0x05
#define IRQ6    0x06
#define IRQ7    0x07
#define IRQ8    0x08
#define PS2PORT 0x60

#define NUM_COLS        80
#define NUM_ROWS        25
#define MAX_CHARACTERS  127
#define LSB8            0xFF

typedef struct s_info{
	int s_esp;
	int s_ebp;
	tss_t s_tss;
} schedule_info;

schedule_info schedule_info_arr[3];

volatile int terminal_request;
volatile int schedule_tick;

int timer_ticks;
int current_process;
int last_esp;

int last_ebp;
int strand_type_lock;

#endif
