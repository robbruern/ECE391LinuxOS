/* syscalls.c - System Call Handler
 * vim:ts=4 noexpandtab
 */
#include "syscalls.h"
#include "filesystem_driver.h"
#include "rtc_driver.h"
#include "interrupts.h"
#include "x86_desc.h"
#include "process_control.h"


extern void init_control_registers_paging(int * ptr);
extern void set_control_registers_paging(int * ptr);
extern void flush_tlb();


/*
 * do_call
 *   DESCRIPTION: 	Handles a Syscall (INT 0x80) with arguments in C format.
 *   INPUTS: 		int call - Syscall number to run.
 * 					int arg1 - First Argument to syscall.
 * 					int arg2 - Second argument to syscall.
 * 					int arg3 - Third argument to syscall.
 *   OUTPUTS:  		None.
 *   RETURN VALUE: 	The return value of the syscall.
 *   SIDE EFFECTS: 	Requisite syscall is run.
 */
int do_call(int call, int arg0, int arg1, int arg2)
{
    int ret;
    ret = -1;

    // Save used registers to stack
    __asm__("pushl %%ebx;" // Callee saved.
            "pushl %%esi;" // Caller saved.
            "pushl %%edi;" // Caller saved.
            :
            :);

    // Move arguments into registers for Syscall INT 0x80

 	__asm__("movl %0, %%eax;"
			"movl %1, %%ebx;"
			"movl %2, %%ecx;"
			"movl %3, %%edx;"
			:
			: "r"(call),"r"(arg0), "r"(arg1), "r"(arg2), "r"(0));

    // Run handle_system_calls. Return value should reside in %eax.
    __asm__("int $0x80;");

    // Restored used registers to stack
    __asm__("popl %%edi;" // Caller saved.
            "popl %%esi;" // Caller saved.
            "popl %%ebx;" // Callee saved.
            :
            :);

    __asm__("movl %%eax, %0;"
            : "=r"(ret)
            :);

    return ret;
}

/*
 * halt
 *   DESCRIPTION: Halts the current process.
 *   INPUTS: uint8_t status - return value for the execute call
 *   OUTPUTS: uint8_t status to execute call.
 *   RETURN VALUE: Never returns
 *   SIDE EFFECTS: Closes the current PCB and restores parent PCB.
 */
int32_t halt(uint8_t status)
{
  int i;
	cli();
	int parent_pid = control_blocks[current_pid].parent;
	past_pid = current_pid;
	// Restore Parent PCB
	destroy_pcb(current_pid);
    current_pid = parent_pid;
    tss.esp0 = control_blocks[current_pid].esp;
for(i = 2; i < FDT_SIZE; i++){
    pcb_close(i);
  }
	process_pages[VIRTUAL_PROGRAM] = (M_4*(current_pid + 1)) | PAGE_ARGS;
	flush_tlb();
	// Move status into EAX.
	__asm__("xorl %%eax, %%eax;"
			"movb %0, %%al;"
            :
            : "r" (status)
            );

	// Jump to return in execute.
	__asm__("jmp HALTED;");

    // Never reaches here...
    return 0;
}

/*
 * schedule
 *   DESCRIPTION: Schedules the current running
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE:
 *   SIDE EFFECTS: Resets current context's top ebp and esp, as well as
 *                 decreases the current process
 */
void schedule()
{
	// Increment the schedule_top index and insert the pid.
	schedule_top[current_terminal]++;
	int top = schedule_top[current_terminal] - 1;
	schedule_stack[current_terminal][top] = current_pid;
}


/*
 * deschedule
 *   DESCRIPTION: Deschedules the current running
 *   INPUTS:
 *   OUTPUTS: none
 *   RETURN VALUE:
 *   SIDE EFFECTS: Resets current context's top ebp and esp, as well as
 *                 decreases the current process
 */

void deschedule()
{
	// Clear all schedule structs associated with this process.
	int top = schedule_top[current_process] - 1;
	schedule_stack[current_process][top] = 0;
	schedule_ebp[current_process][top] = 0;
	schedule_esp[current_process][top] = 0;
	schedule_top[current_process]--;
}


/*
 * execute
 *   DESCRIPTION: Runs an executable based on which command is passed in
 *   INPUTS: const uint_8 * command: A string containing the name of the program to be run
 *   OUTPUTS: none
 *   RETURN VALUE: Returns the status
 *   SIDE EFFECTS: sets pcb and opens selected file
 */
int32_t execute(const uint8_t *command)
{
	/* Initialize vars */
	int execute_return = 0;
	int temp = 0;
	int fd = 0;
	uint32_t entry = 0;
    int i = 0;
	int magic = 0;
	/* 10000 is an arbitrarily large buffer*/
    int buf[10000];
    int length = 10000;
    uint8_t cmd[EXEC_BUFFER_SIZE] = {0};
    int addr;

	// null check.
    if (command == NULL) {
      	return -1;
    }

	/*If current pid is at the max*/
	if (num_active_processes >= MAX_PROCESSES) {
		return -1;
	}

	// Save the stack position in temp
	__asm__("movl %%esp, %0"
            : "=r" (temp)
            );

    // Enable Interrupts
	/* If the command is too long, fail */
	if (strlen((const int8_t*)command) >= EXEC_BUFFER_SIZE) {
		return FAILURE;
	}

	/*Copy the command into the array cmd*/
	i = 0;
	while (command[i] != ' ' && command[i] != '\0') {
		cmd[i] = command[i];
		i++;
	}
	cmd[i] = '\0';
  // Save i for passing args later.

	// Attempt to open the cmd
	fd = open(cmd);
	if (fd == FAILURE) {
		return FAILURE;
	}

	/*Read the contents into buf*/
	read(fd, buf, length);
	close(fd);

	/*copy the first four bytes of the buf into magic*/
	memcpy(&magic,buf,4);

	/*if magic is not the magic leading numbers, fail*/
	if (magic != MAGIC_LEAD) {
		return FAILURE;
	}

	// Set up new page
	current_pid = create_new_pcb(current_pid);
	control_blocks[current_pid].stack_pos = temp;

	if (0 == strncmp((int8_t *)cmd, (int8_t *)"shell", strlen("shell")))
	{
		if (strlen((int8_t *)cmd) == strlen((int8_t *)"shell"))
		{
			shell_pid[current_terminal] = current_pid;
		}
	}

	/*Calculate the espo by taking the kernel address and adding
	 * the 4 mb offset and subtracting 15 and then subtract from there
	 * 8 kilobytes times the current pid*/
	tss.esp0 = KERNEL_ADDR + (M_4 - 0xF) - (2 * K_4 * current_pid);
	tss.ss0 = KERNEL_DS;

	/*Load the new pcv with the new ss0 and esp0*/
	load_pcb(current_pid, KERNEL_DS, tss.esp0);
	  // Move args.
    // Remove leading spaces.
    while (command[i] == ' ')
    {
        i++;
    }

    // If no args passed, clear args. else, copy args.
    if (command[i] == '\0') {
        clear_args();
    }
    else {
        strncpy((int8_t *)control_blocks[current_pid].args,
            (const int8_t *)&command[i], (uint32_t)strlen((const int8_t *)&command[i]));
    }

    // intializes paging and don't need to do anything for 8Mb to 4Gb
    // intializes first table in mem
	i = 0;
	for (i = 0; i < PAGE_SIZE; i++)
    {
		process_pages[i] = READWRITE_MASK;
		process_tables[i] =  READWRITE_MASK;
	}

    // maps the kernel, sets it to present
	process_pages[1] = KERNEL_USER;

	process_tables[VIDEOMEM] = VIDEO | READWRITE_MASK | PRESENT_MASK;

	process_tables[0xB9] = VIDEO_1 | USER_MASK | READWRITE_MASK | PRESENT_MASK;

	process_tables[0xBA] = (VIDEO_1 + 0x1000) | USER_MASK | READWRITE_MASK | PRESENT_MASK;

	process_tables[0xBB] = (VIDEO_1 + (0x1000 * 2)) | USER_MASK | READWRITE_MASK | PRESENT_MASK;
	// sets up first table in the directory
	process_pages[0] = ((unsigned int)process_tables) | USER_MASK | READWRITE_MASK | PRESENT_MASK;

	/*Set the location of the process to the location of the pid+ 1 times the size of the
	 * kernel*/
	process_pages[VIRTUAL_PROGRAM] = (M_4*(current_pid + 1)) | PAGE_ARGS;

    /*Create table entry for video mem*/
	process_tables[VID_IDX] = (VIDEO) | USER_MASK | READWRITE_MASK | PRESENT_MASK;
	addr = (int)&process_tables[VID_IDX];
	process_pages[VID_IDX] = ((addr >> 12) << 12) | 0x7;
	// clear keyboard driver data.
	clear_keyboard_buffer();
	clear_history_buffer(current_pid);

	/*Init paging to have the new page directory*/
	flush_tlb();
	video_mem = (char *)VIDMAP;
	/*Copy the program from the buffer and into the virtual mem*/
	memcpy((void*)(VIRTUAL_START), buf, length); // 128MB VM

	/*Get the entry point of the executable, it is four bytes in size
	 * so we copy the four bytes*/
	memcpy((void*)&entry, (const void*) ENTRY_START, 4);

	if (terminal_running[current_terminal] == 0) {
		terminal_running[current_terminal] = 1;
	}

	schedule();
	__asm__("movl %0, %%ds"
            :
            : "r" (USER_DS)
            );

	__asm__("pushl %0;"
            "pushl %1;"
			"pushf;"
			"pushl %2;"
			"pushl %3;"
            :
            : "r" (USER_DS), "r" (STACK_LOCATION) , "r" (USER_CS), "r" (entry)
            );

	__asm__("iret");
	// Switch back
    __asm__ ("HALTED:;");

	__asm__("movl %%eax, %0"
            : "=r" (execute_return)
            );

	temp = control_blocks[past_pid].stack_pos;
	__asm__("movl %0, %%esp"
            :
			: "r" (temp)
            );

	deschedule();


	sti();


	if (current_pid == SENTINEL_PROCESS) {
		do_call(SYS_EXECUTE, (int) "shell", 0, 0);
	}

	return execute_return;
}


/*
 * read
 *   DESCRIPTION: Calls the correct read function based on file type
 *   INPUTS: fd -  file table idx to read from
 *					 buf - buffer that will fill with read characters
 *           nbytes - number of bytes to write
 *   OUTPUTS: None
 *   RETURN VALUE: number of bytes read on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
	/* NULL check */
	cli();
	if (buf == NULL) {
		return -1;
	}

	if (fd < 0 || fd >= FDT_SIZE) {
		return -1;
	}

	if (control_blocks[current_pid].fd_table[fd].flags == -1) {
		return -1;
	}
    if(control_blocks[current_pid].fd_table[fd].file_operations_pointer == NULL){
        return -1;
    }
	return control_blocks[current_pid].fd_table[fd].file_operations_pointer->read(fd, buf, nbytes);
}

/*
 * write
 *   DESCRIPTION: Calls the correct write function based on file type
 *   INPUTS: fd -  file table idx to write to
 *					 buf - buffer containing characters to write
 *           nbytes - number of bytes to write
 *   OUTPUTS: None
 *   RETURN VALUE: number of bytes written on success, -1 on failure
 *   SIDE EFFECTS: writes to a file
 */
int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    // sti();
	cli();
	if (buf == NULL) {
		return -1;
    }
	if (fd < 0 || fd >= FDT_SIZE) {
		return -1;
	}

	if (control_blocks[current_pid].fd_table[fd].flags == -1) {
		return -1;
	}
	if (control_blocks[current_pid].fd_table[fd].file_operations_pointer == NULL) {
		return -1;
	}
    return control_blocks[current_pid].fd_table[fd].file_operations_pointer->write(fd, buf, nbytes);
}

/*
 * open
 *   DESCRIPTION: Calls the correct open function based on file type
 *                if there is an open spot in fdtable, will take it
 *   INPUTS: filename -  file name to open
 *   OUTPUTS: none
 *   RETURN VALUE: ret - returns fdtable index on success, -1 on failure
 *   SIDE EFFECTS: sets file_operations_pointer, inode, file posiion, and flags
 *                 removes a spot from fdtable if taken
 */

int32_t open(const uint8_t *filename)
{
  int ret;
	dir_entry_t dentry; // used to store inodes in pcb
	int file_type;
  cli();
  if(filename == NULL){
    return -1;
  }

	if (read_dentry_by_name(filename, &dentry) != 0) {
		ret = FAILURE;
		return ret;
	}
	file_type = dentry.file_type;
	switch(file_type)
	{
		case TYPE_RTC:
            ret = rtc_open(filename);
			break;
		case TYPE_DIR:
			ret = directory_open(filename);
			break;
		case TYPE_FILE:
			ret = file_open(filename);
			break;
	}
	return ret;
}

/*
 * close
 *   DESCRIPTION: Calls the correct open function based on file type
 *   INPUTS: fname -  file name to open
 *					 file_type - type of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: ret - returns PCB index on success, -1 on failure
 *   SIDE EFFECTS: sets pcb and opens selected file
 */
int32_t close(int32_t fd)
{
	cli();
  	// Reject closing stdin and stdout
    if (fd < 2 || fd >= FDT_SIZE) {
      return -1;
    }
    if (control_blocks[current_pid].fd_table[fd].file_operations_pointer == NULL) {
      return -1;
    }
    return control_blocks[current_pid].fd_table[fd].file_operations_pointer->close(fd);
}

/*
 * getargs
 *   DESCRIPTION: Reads the commandline arguments a
 *   INPUTS:
 *			 buf - buffer that will fill with read characters
 *           nbytes - number of bytes to write
 *   OUTPUTS: None
 *   RETURN VALUE: number of bytes read on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t getargs(void *buf, int32_t nbytes)
{
	/* NULL check */
	cli();
	if (buf == NULL) {
		return -1;
	}
  if(nbytes <= 0){
    return -1;
  }
	// Cast void * param as character buffer.
	int8_t * buffer = (int8_t *) buf;

	// Check argument size with nbytes size.
    int args_size = strlen((const int8_t *)control_blocks[current_pid].args);
    if (args_size > nbytes || args_size <= 0) {
		return FAILURE;
	}

	// Copy arguments into buffer and null-terminate.
    strncpy(buffer, (const int8_t *)control_blocks[current_pid].args, args_size);
	buffer[args_size] = '\0';

	// Clear argument buffer and return success.
    clear_args();
    return SUCCESS;
}

/*
 * vidmap
 *   DESCRIPTION: Calls the correct open function based on file type
 *   INPUTS: screen_start
 *   OUTPUTS: none
 *   RETURN VALUE: ret - returns PCB index on success, -1 on failure
 *   SIDE EFFECTS: sets up user memory
 */
int32_t vidmap(uint8_t **screen_start)
{
	cli();
    // sti();
    if (screen_start == NULL) {
      return -1;
    }
	if (screen_start == 0) {
		return FAILURE;
	}
	if (screen_start >= (uint8_t**)PROGMEM_LOWER && screen_start <= (uint8_t**)PROGMEM_UPPER) {
		return FAILURE;
	}

	*screen_start = (uint8_t*)VIDMAP;

	return SUCCESS;
}

/*
 * set_handler
 *   DESCRIPTION: Calls the correct open function based on file type
 *   INPUTS: fname -  file name to open
 *					 file_type - type of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: ret - returns PCB index on success, -1 on failure
 *   SIDE EFFECTS: sets pcb and opens selected file
 */
int32_t set_handler(int32_t signum, void *handler_address)
{
	  cli();
    if(handler_address == NULL) {
		return -1;
	}
    return -1;
}

/*
 * sigreturn
 *   DESCRIPTION: Calls the correct open function based on file type
 *   INPUTS: fname -  file name to open
 *					 file_type - type of file to open
 *   OUTPUTS: none
 *   RETURN VALUE: ret - returns PCB index on success, -1 on failure
 *   SIDE EFFECTS: sets pcb and opens selected file
 */
int32_t sigreturn(void)
{
	  cli();
    return -1;
}
/*
 * pcb_close
 *   DESCRIPTION: closes a file in the current pcb
 *   INPUTS: fd - file to close
 *
 *   OUTPUTS: none
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: closes a given fd file
 */
void pcb_close(int fd){
  //TODO: Finish function
  return;
}
