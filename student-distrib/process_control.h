/* process_control.h - Process controller.
 * vim:ts=4 noexpandtab
 */
#pragma once
#include "x86_desc.h"
#include "interrupts.h"
#include "filesystem_driver.h"

#define M_4 0x400000  // Memory
#define K_4 0x4000    // Kernel

#define KERNEL_ADDR     0x400000  // Kernel Address
#define PROGRAM_ADDR    0x400000  // Program Address
#define PROGRAM_OFFSET  0x48000   // Program Address Offset
#define ENTRY_OFFSET    0x48018   // Memory Entry Offset

#define SENTINEL_PROCESS 0    // Process ID for the sentinel/root parent.
#define TOTAL_PROCESSES 7     // Total number of processes - including sentinel.
#define MAX_PROCESSES   6     // Maximum number of processes.
#define FDT_SIZE        8     // Process control block Size.
#define PAGE_SIZE       1024  // Page size for our paging
#define EXEC_BUFFER_SIZE 128
#define ARGS_BUFFER_SIZE 1024

// Operation Table Structure.
typedef struct optable
{
    int32_t (*open)(const uint8_t * filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes );
    int32_t (*close)(int32_t fd);
} optable_t;

// File Descriptor Block Structure.
typedef struct fd_block
{
    optable_t *file_operations_pointer;
    int32_t inode;
    uint32_t file_position;
    int32_t flags;
} fd_block_t;

// Process Control Block Structure.
typedef struct process_control_block_ {
    fd_block_t fd_table[FDT_SIZE];
    int pid;
    int parent;
    int stack_pos;
    int ss;
    int esp;
    uint8_t args[ARGS_BUFFER_SIZE];
} pcb_t;

// The Global Process Control Blocks and Process ID.
pcb_t control_blocks[TOTAL_PROCESSES];
int num_active_processes;
int current_pid;
int past_pid;

// Operation table global variables.
optable_t file_table;
optable_t * file;
optable_t dir_table;
optable_t * dir;
optable_t stdin_table;
optable_t * stdin;
optable_t stdout_table;
optable_t * stdout;

// Per-process page directories and page tables.
uint32_t process_pages/*[TOTAL_PROCESSES]*/[PAGE_SIZE] __attribute__((aligned(4 * PAGE_SIZE)));
uint32_t process_tables/*[TOTAL_PROCESSES]*/[PAGE_SIZE] __attribute__((aligned(4 * PAGE_SIZE)));

extern int create_new_pcb(int parent);
extern void load_pcb(int pid, int ss, int esp);

extern int destroy_pcb(int pid);
extern int process_control_block_init();
extern int first_process_init();
extern void clear_args();
