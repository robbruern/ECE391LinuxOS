/* terminal.c - Terminal driver
 * vim:ts=4 noexpandtab
 */
#include "terminal.h"
volatile unsigned char return_switch[NUM_TERMINALS];
unsigned char read_buffer[NUM_TERMINALS][MAX_BUFFER_SIZE];
unsigned char cursor_location[MAX_PROCESSES];
uint8_t vid_save_arr[NUM_TERMINALS][NUM_COLS * NUM_ROWS];

/* void terminal_switch()
 * Description: Switches to another terminal.
 * Inputs:      uint8_t num;
 * Outputs:     NONE
 * Return Value: NONE.
 * Side Effects:  NONE
 */
int32_t terminal_switch(uint8_t num)
{
	int previous_terminal = 0;
	if (num > NUM_TERMINALS)
	{
		return FAILURE;
	}

	// Already on current terminal.
	if (num == current_terminal)
	{
		return SUCCESS;
	}

	if (num_active_processes >= MAX_PROCESSES && terminal_running[num] == 0)
	{
		return SUCCESS;
	}
	cli();
	switch_data_arr[current_terminal].cursor_x = get_screen_x();
	switch_data_arr[current_terminal].cursor_y = get_screen_y();

	previous_terminal = current_terminal;
	current_terminal = num;

	if (terminal_running[current_terminal] == 1)
	{

		memcpy((char *)VIDEO_1 + (0x1000 * previous_terminal), (char *)VIDEO, 0x1000);
		memcpy((char *)VIDEO, (char *)VIDEO_1 + (0x1000 * current_terminal), 0x1000);

		set_screen_x(switch_data_arr[current_terminal].cursor_x);
		set_screen_y(switch_data_arr[current_terminal].cursor_y);
		cursor_update();

		terminal_request = current_terminal;
		int schedule_ticked = schedule_tick;
		send_eoi(IRQ1);
		sti();
		while (schedule_ticked == schedule_tick) {}
		cli();
	}
	else
	{

		memcpy((char *)(VIDEO_1 + (0x1000 * previous_terminal)), (char *)VIDEO, 0x1000);
		memcpy((char *)VIDEO, (char *)(VIDEO_1 + (0x1000 * current_terminal)), 0x1000);

		terminal_clear();
		send_eoi(IRQ1);
		current_pid = SENTINEL_PROCESS;
		current_process = current_terminal;

		do_call(SYS_EXECUTE, (int)"shell", 0, 0);
	}
	sti();

	return SUCCESS;
}

/* void terminal_open()
 * Description: "Opens" the terminal driver.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: SUCCESS if success, FAILURE if failure.
 * Side Effects:  NONE
 */
int32_t terminal_open(const uint8_t* filename) {
	int i = 0;
	int j = 0;
	int m = 0;

	// int n = 0;
	clear_all_read_buffers();

	/* initialize current_terminal to zero*/
	current_terminal = 0;


	/*Clear the vid memory save array*/
	for (i = 0; i < NUM_TERMINALS; i++)
	{
		return_switch[i] = 0;

		terminal_running[i] = 0;
		switch_data_arr[i].cursor_x = 0;
		switch_data_arr[i].cursor_y = 0;

		schedule_top[i]= 0;

		for (j = 0; j < 7; j++)
		{
			schedule_stack[i][j] = 0;

			schedule_ebp[i][j] = 0;
			schedule_esp[i][j] = 0;
		}

		for (m = 0; m < NUM_ROWS * NUM_COLS; m++)
		{
			*(uint8_t *)(switch_data_arr[i].vid_data + (m << 1)) = ' ';
			*(uint8_t *)(switch_data_arr[i].vid_data + (m << 1) + 1) = 0x7;
		}

	}

	return SUCCESS;
}
/* void terminal_close()
 * Description: "Closes" the keyboard driver.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Clears all keyboard buffers.
 */
int32_t terminal_close(int32_t fd) {
	control_blocks[current_pid].fd_table[1].file_operations_pointer = NULL;
	control_blocks[current_pid].fd_table[1].inode = -1;
	control_blocks[current_pid].fd_table[1].flags = -1;
	return SUCCESS;
}

/* void terminal_write(const void* buf, int nbytes)
 * Description: Writes/prints a given string to the terminal.
 * Inputs:      int32_t fd - file descriptor. Unused.
 * 				const void * buf - String buffer to write.
 * 				int32_t nbytes - number of characters to write.
 * Outputs:     NONE
 * Return Value: The amount of characters written.
 * Side Effects:  	Prints a given string to the display.
 * 					Will move to next line/scroll if necessary.
 */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes)
{
	// Parameter check.
	if (nbytes < 1) {
		return FAILURE;
	}
	cli();
	char * output;
	output = (char *) buf;

	int32_t i = 0;
	while (i < nbytes) {
		// Keep track of if next line is after output
		unsigned char call_next_line = 0;
		if (get_screen_x() == NUM_COLS - 1)
		{
			call_next_line = 1;
		}

		// Fetch from buffer and putc.
		// If the character is a newline, move to next line.
		unsigned char c = output[i];
		if (c == '\n') {
			call_next_line = 1;
		} else {
			putc(c);
				if(key_flag == 0){
			}
			else{
				key_flag = 0;
			}
		}
		// Update display appropriately
		if (call_next_line == 1)
		{
			sti();
			next_line();
			cli();
		}
		i++;
	}

	cursor_update();

	send_eoi(IRQ1);
	sti();
	return i;
}

/* int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Description: Waits for an return key press, and then copies the keyboard buffer to a given buffer.
 * Inputs:      int32_t fd - File descriptor. Unusused.
 * 				void * buf - Buffer to copy keyboard buffer to.
 * 				int32_t nbytes - Number of bytes to copy.
 * Outputs:     NONE
 * Return Value: From keyboard_read - Number of characters/bytes copied.
 * Side Effects: Requires interrupts from the keyboard.
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
{
	int byte_cnt;
	int i;
	byte_cnt = 0;
	// Parameter check.
	if (nbytes < 1) {
		return FAILURE;
	}
	if( buf == NULL){
		return FAILURE;
	}
	int cip = current_process;
	// Set return switch to on.
	return_switch[cip] = SWITCH_ON;

	// Wait for return signal.
	sti();
	while (return_switch[cip] != SWITCH_OFF) {}
	cli();

	// Set null termination on last character, then copy to destination.
	read_buffer[cip][nbytes - 1] = '\0';

	i = 0;
	while (read_buffer[cip][i] != '\n') {
		i++;
	}
	strncpy((char *) buf, (char *) read_buffer[cip], nbytes);

	// Carriage return.
	next_line();
	cursor_update(); // Update cursor

	send_eoi(IRQ1);
	sti();

	return i + 1;
}

/* int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Description: Waits for an return key press, and then copies the keyboard buffer to a given buffer.
 * Inputs:      int32_t fd - File descriptor. Unusused.
 * 				void * buf - Buffer to copy keyboard buffer to.
 * 				int32_t nbytes - Number of bytes to copy.
 * Outputs:     NONE
 * Return Value: From keyboard_read - Number of characters/bytes copied.
 * Side Effects: Requires interrupts from the keyboard.
 */
int32_t terminal_read_fail(int32_t fd, void* buf, int32_t nbytes)
{
	// You should not be able to read from stdin.
	return FAILURE;
}

/* void terminal_print(char * string)
 * Description: Wrapper function to terminal_write, with simplified parameters.
 * Inputs:      char * string - the input string.
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Prints the given string to the terminal.
 */
void terminal_print(char * string) {
	terminal_write(1, string, strlen(string));
}

/* void terminal_backspace()
 * Description: Handles a backspace input from keyboard as it relates to the terminal.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Removes a character at the screen position.
 */
void terminal_backspace() {
	backspace();
	cursor_update();
}

/* void terminal_return()
 * Description: Handles a return input from keyboard as it relates to the terminal.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Moves to next line, scrolls if necessary.
 */
void terminal_return() {
	if (return_switch[current_terminal] == SWITCH_ON) {
		keyboard_read(1, read_buffer[current_terminal], MAX_BUFFER_SIZE);
		set_return_switch(0);
	} else if (return_switch[current_terminal] == SWITCH_OFF) {
		// Scroll if near bottom of page. Else, next line.
		next_line();
		cursor_update(); // Update cursor
	}
}

/* unsigned char get_return_switch()
 * Description: Passes the current state of return_switch.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: The current state of return_switch.
 * Side Effects:  NONE
 */
unsigned char get_return_switch() {
	return return_switch[current_process];
}

/* void get_return_switch()
 * Description: Flips the current state of return_switch.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Return switch will now be the opposite boolean value.
 */
void flip_return_switch() {
	if (return_switch[current_terminal] == 0) {
		return_switch[current_terminal] = 1;
	} else {
		return_switch[current_terminal] = 0;
	}
}

/* void get_return_switch()
 * Description: Sets the current state of return_switch.
 * Inputs:      unsigned char value - the value to set switch to (only 0 or 1).
	tss.ss0 = KERNEL_DS;
	tss.esp0 = KERNEL_ADDR + (M_4 - 0xF) - (2 * K_4 * current_pid);
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Return switch will now be the passed boolean value, if valid.
 */
void set_return_switch(unsigned char value) {
	if (value == 0 || value == 1) {
		return_switch[current_terminal] = value;
	}
}

/* void set_return_switch_spec()
 * Description: Sets the current state of return_switch.
 * Inputs:      unsigned char value - the value to set switch to (only 0 or 1).
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Return switch will now be the passed boolean value, if valid.
 */
void set_return_switch_spec(unsigned char terminal, unsigned char value) {
	if (terminal < NUM_TERMINALS) {
		if (value == SWITCH_OFF || value == SWITCH_ON || value == SWITCH_HOLD) {
			return_switch[terminal] = value;
		}
	}
}

/* void terminal_clear()
 * Description: Handles a terminal clear input from the keyboard.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Clears display and resets screen position to top left corner.
 */
void terminal_clear() {
	clear(); 			// Clears display.
	set_screen_x(0);	// Sets screen indices.
	set_screen_y(0);
	cursor_update(); // Updates cursor.
}

/*
 * reset_terminal_keyboard_input(int index, int size)
 *   DESCRIPTION: Clears the displayed keyboard buffer.
 *   INPUTS:
 *
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: display is cleared.
 */
void reset_terminal_keyboard_input(int index, int size)
{
	cursor_update_right(size - index);
	int i;
	for (i = size; i > 0; i--)
	{
		terminal_backspace();
	}
}

/* void clear_read_buffer()
 * Description: Initializes/clears the read_buffer()
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Sets the read_buffer all to null characters.
 */
void clear_read_buffer() {
	// clear buffer and buffer index.
	int i;
	for (i = 0; i < MAX_BUFFER_SIZE; i++)
	{
		read_buffer[current_terminal][i] = '\0';
	}
}

/* void clear_read_buffer()
 * Description: Initializes/clears the read_buffer()
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Sets the read_buffer all to null characters.
 */
void clear_all_read_buffers()
{
	// clear buffer and buffer index.
	int j;
	for (j = 0; j < NUM_TERMINALS; j++)
	{
		int i;
		for (i = 0; i < MAX_BUFFER_SIZE; i++)
		{
			read_buffer[current_terminal][i] = '\0';
		}
	}
}

/*
 * cursor_enable_spec
 *   DESCRIPTION: 	Intializes use of the VGA Text Mode cursor.
 *   INPUTS: 		unsigned char top_scanline - scanline index from top to allow cursor.
 * 					unsigned char bottom_scanline - scanline index from bottom to allow cursor.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flashing cursor is displayed in given region.
 */
void cursor_enable_spec(unsigned char top_scanline, unsigned char bottom_scanline) {
	outb(0x0a, 0x3D4);
	outb((inb(0x3D5) & 0xC0) | top_scanline, 0x3D5);

	outb(0x0B, 0x3D4);
	outb((inb(0x3D5) & 0xE0) | bottom_scanline, 0x3D5);
}

/*
 * cursor_enable
 *   DESCRIPTION: 	Intializes use of the VGA Text Mode cursor.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flashing cursor is displayed in the terminal region (0-25).
 */
void cursor_enable() {
	outb(0x0A, 0x3D4);
	outb(0x00, 0x3D5);
	outb(0x0B, 0x3D4);
	outb(NUM_ROWS, 0x3D5);
}

/*
 * cursor_disable
 *   DESCRIPTION: 	Disables/clears the use of the VGA Text Mode cursor.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flashing cursor is no longer displayed.
 */
void cursor_disable() {
	outb(0x0A, 0x3D4);
	outb(0x20, 0x3D5);
}

/*
 * cursor_update_spec
 *   DESCRIPTION: 	Updates the position of the VGA Text mode cursor.
 *   INPUTS: 		int x_position - index to the 'row' position of the cursor.
 *   				int y_position - index to the 'column' position of the cursor.
 * 	 OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Flashing cursor is now displayed at given location.
 */
void cursor_update_spec(int x_position, int y_position) {
	unsigned short screen_position = (y_position * NUM_COLS) + x_position;
	outb(0x0E, 0x3D4);
	outb((unsigned char) ((screen_position >> 8) & LSB8), 0x3D5);
	outb(0x0F, 0x3D4);
	outb((unsigned char) (screen_position & LSB8), 0x3D5);
}

/*
 * cursor_update
 *   DESCRIPTION: 	Automatically updates the position of the VGA Text mode cursor.
 *   				Based on the current screen position.
 * 	 INPUTS: 		none
 * 	 OUTPUTS: 		none
 *   RETURN VALUE: 	none
 *   SIDE EFFECTS: 	Flashing cursor is now displayed at current screen location.
 */
void cursor_update() {
	cursor_update_spec(get_screen_x(), get_screen_y());
}

/*
 * cursor_update_left
 *   DESCRIPTION: 	Automatically updates the position of the VGA Text mode cursor.
 *   				Moves the current screen position left by amount of cells.
 * 	 INPUTS: 		int amount - amount to move cursor left by
 * 	 OUTPUTS: 		none
 *   RETURN VALUE: 	none
 *   SIDE EFFECTS: 	Flashing cursor is now displayed at current screen location.
 */
void cursor_update_left(int amount) {
	set_screen_x(get_screen_x() - amount);
	cursor_update();
}

/*
 * cursor_update_right
 *   DESCRIPTION: 	Automatically updates the position of the VGA Text mode cursor.
 *   				Moves the current screen position right by amount of cells.
 * 	 INPUTS: 		int amount - amount to move cursor right by
 * 	 OUTPUTS: 		none
 *   RETURN VALUE: 	none
 *   SIDE EFFECTS: 	Flashing cursor is now displayed at current screen location.
 */
void cursor_update_right(int amount) {
	set_screen_x(get_screen_x() + amount);
	cursor_update();
}

/*
 * cursor_position
 *   DESCRIPTION: 	Acquires the cursor position from VGA.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: The cursor position as row major order index.
 *   SIDE EFFECTS: none
 */
unsigned short cursor_position() {
	unsigned short screen_position = 0;

	outb(0x0F, 0x3D4);
	screen_position |= ((unsigned short) inb(0x3D5));

	outb(0x0E, 0x3D4);
	screen_position |= ((unsigned short) inb(0x3D5)) << 8;

	return screen_position;
}
