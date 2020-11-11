/* keyboard.h - Keyboard driver
 * vim:ts=4 noexpandtab
 */
#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#pragma once

#include "types.h"
#include "lib.h"
#include "terminal.h"
#include "filesystem_driver.h"
#include "syscalls.h"

#define PAGE_SIZE 1024

#define LSHIFTPRESS 0x36
#define RSHIFTPRESS 0x2A
#define LSHIFTRELEASE 0xB6
#define RSHIFTRELEASE 0xAA
#define CAPSLOCK 0x3A
#define RELEASE 0x80
#define SHIFT 90
#define ENTER 0x1C
#define CTRLPRESS 0x1D
#define ALTPRESS 0x38
#define ALTRELEASE 0xB8
#define L 0x26
#define C 0x2E
#define D 0x20
#define CTRLRELEASE 0x9D
#define BACKSPACE 0x0E
#define UP_ARROW 0x48
#define DOWN_ARROW 0x50
#define LEFT_ARROW 0x4B
#define RIGHT_ARROW 0x4D

#define ALT_F1 0xF1
#define ALT_F2 0xF2
#define ALT_F3 0xF3
#define ALT_F4 0xF4

#define INVALID_SCANCODE 0

#define MAX_BUFFER_SIZE 128
#define MAX_CHARACTERS 127
#define NUM_HISTORY_BUFFERS 20
int key_flag;
extern int32_t keyboard_open(const uint8_t *filename);
extern int32_t keyboard_write(int32_t fd, const void *buf, int32_t nbytes);
extern int32_t keyboard_read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t keyboard_close(int32_t fd);

extern void copy_to_history();
extern void increment_history_indices();
extern void handle_keyboard_input(unsigned short key);
extern void keyboard_insert(char c);
extern void keyboard_backspace();
extern void up_history();
extern void down_history();
extern void clear_all_keyboard_buffers();
extern void clear_keyboard_buffer();
extern void clear_history_buffer(int pid);
extern void keyboard_left_arrow();
extern void keyboard_right_arrow();

/* Table used for translating keyboard codes */
static const unsigned char scancode[256] =
{
  INVALID_SCANCODE, INVALID_SCANCODE /* Esc */, '1', '2', '3', '4', '5', '6', '7', '8',             /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */ INVALID_SCANCODE,                        /* Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   /* Enter key */
  'c',                                                                /* 29 */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',                   /* 39 */
  '\'', '`', INVALID_SCANCODE,                                                       /* Left shift */
  '\\', 'z', 'x', 'c', 'v', 'b', 'n', /* 49 */ 'm', ',', '.', '/', INVALID_SCANCODE, /* Right shift */
  '*', INVALID_SCANCODE, /* Alt */ ' ', /* Space bar */ INVALID_SCANCODE,                           /* Caps lock */
  INVALID_SCANCODE, /* 59 - F1 key */ INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,                     /* F10 */
  INVALID_SCANCODE,                                                                  /* 69 ---------- Num Pad below ---------------------------*/
  INVALID_SCANCODE,                                                                  /* Scroll Lock */
  INVALID_SCANCODE, /* Home key or 7 */ INVALID_SCANCODE, /* Up Arrow  or 8*/ INVALID_SCANCODE,                    /* Page Up or 9*/
  '-',
  INVALID_SCANCODE, /* Left Arrow or 4 */ INVALID_SCANCODE, INVALID_SCANCODE, /* Right Arrow or 6*/
  '+',
  INVALID_SCANCODE, /* 79 - End or 1*/ INVALID_SCANCODE, /* Down Arrow or 2 */ INVALID_SCANCODE,                      /* Page Down or 3 */
  INVALID_SCANCODE, /* Insert Key or 0 */ INVALID_SCANCODE,                                            /* Delete Key or . */
  INVALID_SCANCODE, INVALID_SCANCODE, '<', INVALID_SCANCODE, /* F11 Key */ INVALID_SCANCODE,                                         /* F12 Key */
  INVALID_SCANCODE,                                                                     /*Nothing else */
  /*----------------------------- Shift is pressed -------------------------- */
  INVALID_SCANCODE, INVALID_SCANCODE /* Esc */, '!', '@', '#', '$' /* shift+4 */, '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', '\b',                                              /* Backspace */
  INVALID_SCANCODE,                                                                     /* Tab */
  //
  'Q', 'W', 'E', 'R', /* 19 */ 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', /* Enter */
  INVALID_SCANCODE,                                                                         /*Cntrl */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', INVALID_SCANCODE,             /* Left shift */
  '|', 'Z', 'X', 'C', 'V', 'B', 'N',                                         /* 49 */
  'M', '<', '>', '?', INVALID_SCANCODE,                                                     /* Right shift */
  '*',
  INVALID_SCANCODE, /* Alt */ ' ', /* Space bar */ 0, /* Caps lock */
  INVALID_SCANCODE,                                   /* F1 */
   INVALID_SCANCODE, INVALID_SCANCODE,  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  INVALID_SCANCODE, /* F10 */
  INVALID_SCANCODE, /* 69 -------- Num pad below--------- */
  INVALID_SCANCODE, /* Scroll Lock */
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  '-',
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  '+',
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  INVALID_SCANCODE, /* Insert/0 */ INVALID_SCANCODE,       /* Delete/. */
  INVALID_SCANCODE, INVALID_SCANCODE, '>', INVALID_SCANCODE, /* F11 */ INVALID_SCANCODE, /* F12 */
  INVALID_SCANCODE,                         /* Nothing else */
};

static const unsigned char CAPScode[256] =
{
  INVALID_SCANCODE, INVALID_SCANCODE /* Esc */, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */ INVALID_SCANCODE,                           /* Tab */
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',                     /* Enter key */
  'C',                                                                                  /* 29 */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',                                     /* 39 */
  '\'', '`', INVALID_SCANCODE,                                                          /* Left shift */
  '\\', 'Z', 'X', 'C', 'V', 'B', 'N', /* 49 */ 'M', ',', '.', '/', INVALID_SCANCODE,    /* Right shift */
  '*', INVALID_SCANCODE, /* Alt */ ' ', /* Space bar */ INVALID_SCANCODE,               /* Caps lock */
  INVALID_SCANCODE, /* 59 - F1 key */ INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,     /* F10 */
  INVALID_SCANCODE,                                                                             /* 69 ---------- Num Pad below ---------------------------*/
  INVALID_SCANCODE,                                                                             /* Scroll Lock */
  INVALID_SCANCODE, /* Home key or 7 */ INVALID_SCANCODE, /* Up Arrow  or 8*/ INVALID_SCANCODE, /* Page Up or 9*/
  '-',
  INVALID_SCANCODE, /* Left Arrow or 4 */ INVALID_SCANCODE, INVALID_SCANCODE, /* Right Arrow or 6*/
  '+',
  INVALID_SCANCODE, /* 79 - End or 1*/ INVALID_SCANCODE, /* Down Arrow or 2 */ INVALID_SCANCODE, /* Page Down or 3 */
  INVALID_SCANCODE, /* Insert Key or 0 */ INVALID_SCANCODE,                                      /* Delete Key or . */
  INVALID_SCANCODE, INVALID_SCANCODE, '<', INVALID_SCANCODE /* F11 Key */, INVALID_SCANCODE,     /* F12 Key */
  INVALID_SCANCODE,                                                                              /*Nothing else */
  /*----------------------------- Shift is pressed -------------------------- */
  INVALID_SCANCODE, INVALID_SCANCODE /* Esc */, '!', '@', '#', '$' /* shift+4 */, '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', '\b',                                                                           /* Backspace */
  INVALID_SCANCODE,                                                                                   /* Tab */
  //
  'q', 'w', 'e', 'r', /* 19 */ 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',    /* Enter */
  INVALID_SCANCODE,                                                             /*Cntrl */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', INVALID_SCANCODE, /* Left shift */
  '|', 'z', 'x', 'c', 'v', 'b', 'n',                                            /* 49 */
  'm', '<', '>', '?', INVALID_SCANCODE,                                         /* Right shift */
  '*',
  INVALID_SCANCODE, /* Alt */ ' ', /* Space bar */ INVALID_SCANCODE, /* Caps lock */
  INVALID_SCANCODE,                                                  /* F1 */
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  INVALID_SCANCODE, /* F10 */
  INVALID_SCANCODE, /* 69 -------- Num pad below--------- */
  INVALID_SCANCODE, /* Scroll Lock */
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  '-',
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  '+',
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  INVALID_SCANCODE, /* Insert/0 */ INVALID_SCANCODE,                                     /* Delete/. */
  INVALID_SCANCODE, INVALID_SCANCODE, '>', INVALID_SCANCODE, /* F11 */ INVALID_SCANCODE, /* F12 */
  INVALID_SCANCODE,                                                                      /* Nothing else */
};
static const unsigned char altcode[256] =
{
  INVALID_SCANCODE, INVALID_SCANCODE /* Esc */, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
  '9', '0', '-', '=', '\b', /* Backspace */ INVALID_SCANCODE,                           /* Tab */
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',                     /* Enter key */
  'C',                                                                                  /* 29 */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',                                     /* 39 */
  '\'', '`', INVALID_SCANCODE,                                                          /* Left shift */
  '\\', 'Z', 'X', 'C', 'V', 'B', 'N', /* 49 */ 'M', ',', '.', '/', INVALID_SCANCODE,    /* Right shift */
  '*', INVALID_SCANCODE, /* Alt */ ' ', /* Space bar */ INVALID_SCANCODE,               /* Caps lock */
  INVALID_SCANCODE, /* 59 - F1 key */ INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,     /* F10 */
  INVALID_SCANCODE,                                                                             /* 69 ---------- Num Pad below ---------------------------*/
  INVALID_SCANCODE,                                                                             /* Scroll Lock */
  INVALID_SCANCODE, /* Home key or 7 */ INVALID_SCANCODE, /* Up Arrow  or 8*/ INVALID_SCANCODE, /* Page Up or 9*/
  '-',
  INVALID_SCANCODE, /* Left Arrow or 4 */ INVALID_SCANCODE, INVALID_SCANCODE, /* Right Arrow or 6*/
  '+',
  INVALID_SCANCODE, /* 79 - End or 1*/ INVALID_SCANCODE, /* Down Arrow or 2 */ INVALID_SCANCODE, /* Page Down or 3 */
  INVALID_SCANCODE, /* Insert Key or 0 */ INVALID_SCANCODE,                                      /* Delete Key or . */
  INVALID_SCANCODE, INVALID_SCANCODE, '<', INVALID_SCANCODE /* F11 Key */, INVALID_SCANCODE,     /* F12 Key */
  INVALID_SCANCODE,                                                                              /*Nothing else */
  /*----------------------------- Shift is pressed -------------------------- */
  INVALID_SCANCODE, INVALID_SCANCODE /* Esc */, '!', '@', '#', '$' /* shift+4 */, '%', '^', '&', '*', /* 9 */
  '(', ')', '_', '+', '\b',                                                                           /* Backspace */
  INVALID_SCANCODE,                                                                                   /* Tab */
  //
  'q', 'w', 'e', 'r', /* 19 */ 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n',    /* Enter */
  INVALID_SCANCODE,                                                             /*Cntrl */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', INVALID_SCANCODE, /* Left shift */
  '|', 'z', 'x', 'c', 'v', 'b', 'n',                                            /* 49 */
  'm', '<', '>', '?', INVALID_SCANCODE,                                         /* Right shift */
  '*',
  INVALID_SCANCODE, /* Alt */ ' ', /* Space bar */ INVALID_SCANCODE, /* Caps lock */
  ALT_F1,                                                  /* F1 */
  ALT_F2, ALT_F3, ALT_F4, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  INVALID_SCANCODE, /* F10 */
  INVALID_SCANCODE, /* 69 -------- Num pad below--------- */
  INVALID_SCANCODE, /* Scroll Lock */
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  '-',
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  '+',
  INVALID_SCANCODE, INVALID_SCANCODE, INVALID_SCANCODE,
  INVALID_SCANCODE, /* Insert/0 */ INVALID_SCANCODE,                                     /* Delete/. */
  INVALID_SCANCODE, INVALID_SCANCODE, '>', INVALID_SCANCODE, /* F11 */ INVALID_SCANCODE, /* F12 */
  INVALID_SCANCODE,                                                                      /* Nothing else */
};
#endif
