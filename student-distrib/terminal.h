/* terminal.h - Terminal Driver
 * vim:ts=4 noexpandtab
 */
#ifndef _TERMINAL_H
#define _TERMINAL_H

#pragma once

#include "types.h"
#include "lib.h"
#include "rtc_driver.h"
#include "keyboard.h"
#include "filesystem_driver.h"
#include "interrupts.h"
#include "i8259.h"

#define NUM_COLS 		80
#define NUM_ROWS		25
#define LSB8			0xFF

#define SWITCH_OFF  0
#define SWITCH_ON   1
#define SWITCH_HOLD 2

#define NUM_TERMINALS   3

// Process Control Block Structure.
typedef struct s_d {
    uint8_t vid_data[2 * NUM_COLS * NUM_ROWS];
    uint8_t cursor_x;
    uint8_t cursor_y;
} switch_data;

switch_data switch_data_arr[NUM_TERMINALS];

int terminal_running[NUM_TERMINALS];
int current_terminal;
int last_esp;

int switch_ebp;

int switch_esp;

extern int32_t terminal_open(const uint8_t* filename);
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t terminal_close(int32_t fd);

extern int32_t terminal_read_fail(int32_t fd, void *buf, int32_t nbytes);
extern int32_t terminal_switch(uint8_t num);

extern void temrinal_print(char * string);
extern void terminal_backspace();
extern void terminal_return();
extern void terminal_clear();
extern void reset_terminal_keyboard_input(int index, int size);
extern void clear_read_buffer();
extern void clear_all_read_buffers();
extern unsigned char get_return_switch();
extern unsigned char get_return_switch_spec(unsigned char terminal);
extern void flip_return_switch();
extern void set_return_switch(unsigned char value);
extern void set_return_switch_spec(unsigned char terminal, unsigned char value);

extern void cursor_enable_spec(unsigned char top_scanline, unsigned char bottom_scanline);
extern void cursor_enable();
extern void cursor_disable();
extern void cursor_update_spec(int x_position, int y_position);
extern void cursor_update();
extern void cursor_update_left();
extern void cursor_update_right();
extern unsigned short cursor_position();

#endif
