/* process_control.c - Process controller.
 * vim:ts=4 noexpandtab
 */
#include "process_control.h"

/*
 * process_control_block_init()
 *   DESCRIPTION: Initializes the process control blocks for the kernel.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: initializes the TOTAL_PROCESSES process control blocks.
 */
int process_control_block_init()
{
    fd_block_t stdin_block; /** file descriptor block to be assigned */
    fd_block_t stdout_block; /** file descriptor block to be assigned */
    stdout = &stdout_table;
    stdin = &stdin_table;
    stdout->open = &terminal_open;
    stdout->read = &terminal_read_fail;
    stdout->write = &terminal_write;
    stdout->close = &terminal_close;
    stdin->open = &keyboard_open;
    stdin->read = &terminal_read;
    stdin->write = &keyboard_write;
    stdin->close = &keyboard_close;

    stdin_block.file_operations_pointer = stdin;
    stdout_block.file_operations_pointer = stdout;
    stdin_block.inode = 0;
    stdout_block.inode = 0;
    stdin_block.file_position = 0;
    stdout_block.file_position = 0;
    stdin_block.flags = 1;
    stdout_block.flags = 1;

    int pcbIdx = 0;
    for (pcbIdx = 0; pcbIdx < TOTAL_PROCESSES; pcbIdx++)
    {
        control_blocks[pcbIdx].fd_table[0] = stdin_block;
        control_blocks[pcbIdx].fd_table[1] = stdout_block;
    }
    /** no-op, will put stdin and stdout in file descriptor table */

    int i;
    file = &file_table;
    dir = &dir_table;

    file->open = &file_open; /** Assign function pointer */
    file->read = &file_read;
    file->write = &file_write;
    file->close = &file_close;

    dir->open = &directory_open;
    dir->read = &directory_read;
    dir->write = &directory_write;
    dir->close = &directory_close;

    for (pcbIdx = 0; pcbIdx < TOTAL_PROCESSES; pcbIdx++)
    {
        for (i = 2; i < FDT_SIZE; i++)
        {
            /** sets each block to NULL, saying there is no file in there */
            control_blocks[pcbIdx].fd_table[i].flags = -1;
            control_blocks[pcbIdx].fd_table[i].inode = -1;
            control_blocks[pcbIdx].fd_table[i].file_operations_pointer = NULL;
        }

        for (i = 0; i < ARGS_BUFFER_SIZE; i++)
        {
            control_blocks[pcbIdx].args[i] = '\0';
        }
    }

    num_active_processes = 0;

    return SUCCESS;
}

/*
 * first_process_init()
 *   DESCRIPTION: Initializes the first process/sentinel of the kernel.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: The integer process ID of the first process.
 *   SIDE EFFECTS: Initializes the first process of the kernel.
 */
int first_process_init()
{
    int i = 0;
    current_pid = SENTINEL_PROCESS;
    int pid = SENTINEL_PROCESS;
    control_blocks[pid].pid = pid;
    control_blocks[pid].parent = pid;

    __asm__("movl %%ss, %0"
            : "=r" (control_blocks[pid].ss)
            );

    __asm__("movl %%esp, %0"
            : "=r" (control_blocks[pid].esp)
            );
    /*set the pid as a flag for the remaining processes*/
    for (i = 1; i < TOTAL_PROCESSES; i++)
    {
        control_blocks[i].pid = -1;
    }

    return pid;
}

/*
 * create_new_pcb(int parent)
 *   DESCRIPTION: Initializes a new process control block for a new process.
 *   INPUTS: int parent - the parent's process ID.
 *   OUTPUTS: none
 *   RETURN VALUE: The new process ID, or FAILURE if unable to do so.
 *   SIDE EFFECTS: Initializes an available process control block.
 */
int create_new_pcb(int parent/*, int ss, int esp*/)
{
    /*Find available block*/
    int new_pid = -1;

    // Find next available process id.
    int i = 0;
    for (i = 1; i < TOTAL_PROCESSES; i++)
    {
        if (control_blocks[i].pid == -1) {
            new_pid = i;
            break;
        }
    }

    // if none were available, return FAILURE
    if (i == TOTAL_PROCESSES) {
        return FAILURE;
    }

    // printf("i:%d\n",i);
    control_blocks[new_pid].pid = new_pid;
    control_blocks[new_pid].parent = parent;

    for (i = 2; i < FDT_SIZE; i++)
    {
        control_blocks[new_pid].fd_table[i].flags = -1;
        control_blocks[new_pid].fd_table[i].inode = -1;
        control_blocks[new_pid].fd_table[i].file_operations_pointer = NULL;
    }

    num_active_processes++;

    clear_args();

    // printf("control:%d\n",  control_blocks[new_pid].pid);
    /*
    control_blocks[new_pid].ss = ss;
    control_blocks[new_pid].esp = esp;
    */
    return new_pid;
}


/*
 * load_pcb(int pid, int ss, int esp)
 *   DESCRIPTION: Initializes a new process control block for a new process.
 *   INPUTS: int pid - the given process's process ID.
 *           int ss - the stack segment value of the process.
 *           int esp - the stack pointer of the process
 *   OUTPUTS: none
 *   RETURN VALUE: SUCCESS if successful, FAILURE on failure.
 *   SIDE EFFECTS: Set values in a given process control block.
 */
void load_pcb(int pid, int ss, int esp)

{
    control_blocks[pid].ss = ss;
    control_blocks[pid].esp = esp;

}

/*
 * destroy_pcb(int pid)
 *   DESCRIPTION: Clears a given process from the process control block.
 *   INPUTS: int pid - the given process's process ID.
 *   OUTPUTS: none
 *   RETURN VALUE: SUCCESS if successful, FAILURE on failure.
 *   SIDE EFFECTS: Sets the given process control block to available.
 */
int destroy_pcb(int pid)
{
    control_blocks[pid].pid = -1;
    num_active_processes--;
    return SUCCESS;
}


/*
 * clear_args()
 *   DESCRIPTION: Clears the current program's argument buffer.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Clears the current program's argument buffer.
 */
void clear_args()
{
    int i;
    for (i = 0; i < ARGS_BUFFER_SIZE; i++)
    {
        if (control_blocks[current_pid].args[i] == '\0') {
            break;
        } else {
            control_blocks[current_pid].args[i] = '\0';
        }
    }
}
