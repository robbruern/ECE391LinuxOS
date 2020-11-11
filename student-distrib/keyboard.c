/* keyboard.c - Keyboard driver
 * vim:ts=4 noexpandtab
 */
#include "keyboard.h"

static unsigned int shift_down = 0;
static unsigned int caps_lock = 0;
static unsigned int ctrl_down = 0;
static unsigned int alt_down = 0;

static unsigned char buffer[NUM_TERMINALS][MAX_BUFFER_SIZE];
static unsigned char bufferIDX[NUM_TERMINALS];
static unsigned char bufferSIZE[NUM_TERMINALS];

// Structure to hold all history data per terminal.
static unsigned char history[TOTAL_PROCESSES][NUM_HISTORY_BUFFERS][MAX_BUFFER_SIZE];
static int historyIDX[TOTAL_PROCESSES]; // Index of which history string the terminal is currently on.
static int historyBASE[TOTAL_PROCESSES]; // Index of the newest/base entry into history.
static int historySIZE[TOTAL_PROCESSES]; // How many entries are in the history data.

/* void keyboard_open()
 * Description: "Opens" the keyboard driver.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Clears all keyboard buffers.
 */
int32_t keyboard_open(const uint8_t* filename) {
	clear_all_keyboard_buffers();
	key_flag = 0;
	return SUCCESS;
}

/* void keyboard_close()
 * Description: "Closes" the keyboard driver.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Clears all keyboard buffers.
 */
int32_t keyboard_close(int32_t fd) {
	control_blocks[current_terminal].fd_table[0].file_operations_pointer = NULL;
	control_blocks[current_terminal].fd_table[0].inode = -1;
	control_blocks[current_terminal].fd_table[0].flags = -1;
	return SUCCESS;
}

/* void keyboard_write(const void* buf, int nbytes)
 * Description: Function to call when write system call is sent to the keyboard driver
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: Should always return FAILURE
 * Side Effects:  Appends to the keyboard buffer if space allows.
 */
int32_t keyboard_write(int32_t fd, const void* buf, int32_t nbytes) {
	return FAILURE;
}

/* char * keyboard_read()
 * Description: Reads the current buffer of the keyboard driver
 * Inputs:      int32_t fd - File descriptor. Unused.
 * 				void * buf - The buffer to copy the keyboard buffer to.
 * 				int32_t nbytes - Number of bytes to copy.
 * Outputs:     NONE
 * Return Value: Number of characters/bytes copied.
 * Side Effects: Passed buffer is overwritten.
 */
int32_t keyboard_read(int32_t fd, void* buf, int32_t nbytes) {
	// Parameter check.
	if (nbytes < 1) {
		return FAILURE;
	}

	// Cast buffer as character parameter.
	char * output;
	output = (char *) buf;

	// Set up while loop with i index counter.
	int32_t i = 0;
	// Do while i is withing nbytes.
	while (i < nbytes) {
		// Pull appropriate char from buffer and place into param.
		unsigned char c = buffer[current_terminal][i];
		output[i] = c;

		// If the character we added was a line feed/null character,
		// return.
		if (c == '\n' || c == '\0')
		{
			return i;
		}

		i++; // increment index counter.

		// If we are now at MAX_BUFFER_SIZE,
		// place line feed character and return.
		if (i == MAX_BUFFER_SIZE) {
			output[i] = '\n';
			return i;
		}
	}

	return i;
}

/* void copy_to_history()
 * Description: Sets history buffer when enter is pressed
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  If history is full roll back to 0
 */
void copy_to_history()
{
	int displayed_pid = schedule_stack[current_terminal][schedule_top[current_terminal] - 1];

	// Copy keyboard buffer into the history buffer.
	int i;
	for (i = 0; i < MAX_BUFFER_SIZE; i++)
	{
		unsigned char c = buffer[current_terminal][i];
		if (c == '\n' || c == '\0') {
			c = '\0';
		}
		history[displayed_pid][historyBASE[displayed_pid]][i] = c;
	}
}

/* void increment_history_indices()
 * Description: Sets history buffer when enter is pressed
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  If history is full roll back to 0
 */
void increment_history_indices()
{
	int displayed_pid = schedule_stack[current_terminal][schedule_top[current_terminal] - 1];

	// Increment the historyIDX, looping around if necessary.
	historyBASE[displayed_pid] = ((historyBASE[displayed_pid] + 1) % NUM_HISTORY_BUFFERS);
	historyIDX[displayed_pid] = historyBASE[displayed_pid];

	if (historySIZE[displayed_pid] < NUM_HISTORY_BUFFERS) {
		historySIZE[displayed_pid]++;
	}
}

/* void handle_keyboard_input()
 * Description: Called when keyboard interrupt is triggered
 * 							Reads from the keyboard port, and translates
 *							the key using a scancode table. Then puts the
 *							key to the screen.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Pressed key is put into the character stream
 */
void handle_keyboard_input(unsigned short key) {
	unsigned short shiftkey;
	unsigned char c;
	// printf("%x ", key);
	/** Read from the keyboard port and then check for shift or capslock */
	if (key == CAPSLOCK)
	{
		if (caps_lock != 1) {
			caps_lock = 1;
		} else {
			caps_lock = 0;
		}
		return;
	}

	if (key == CTRLPRESS)
	{
		ctrl_down = 1;
		return;
	}

	if (key == CTRLRELEASE)
	{
		ctrl_down = 0;
		return;
	}
	if (key == ALTPRESS)
	{
		alt_down = 1;
		return;
	}

	if (key == ALTRELEASE)
	{
		alt_down = 0;
		return;
	}
	if (key == LSHIFTPRESS)
	{
		if (shift_down != 1) {
			shift_down = 1;
		} else {
			shift_down = 0;
		}
		return;
	}

	if (key == RSHIFTPRESS)
	{
		if (shift_down != 1) {
			shift_down = 1;
		} else {
			shift_down = 0;
		}
		return;
	}

	if (key == LSHIFTRELEASE)
	{
		if (shift_down == 1) {
			shift_down = 0;
		} else {
			shift_down = 1;
		}
		return;
	}

	if ((key == RSHIFTRELEASE))
	{
		if (shift_down == 1) {
			shift_down = 0;
		} else {
			shift_down = 1;
		}
		return;
	}
	if (!(key & RELEASE) && key == UP_ARROW) {
		up_history();
		return;
	}
	if (!(key & RELEASE) && key == DOWN_ARROW) {
		down_history();
		return;
	}

    if (!(key & RELEASE) && key == LEFT_ARROW) {
        keyboard_left_arrow();
        return;
    }

    if (!(key & RELEASE) && key == RIGHT_ARROW) {
        keyboard_right_arrow();
        return;
    }

	if (!(key & RELEASE) && key == BACKSPACE)
	{
		// Already cleared buffer. return.
		if (bufferIDX[current_terminal] == 0) {
			return;
		}

		// Else, perform backspace.
	  	keyboard_backspace();
		return;
	}

	if(!(key & RELEASE) && key == ENTER)
	{
		if (buffer[current_terminal][0] != '\0') {
			copy_to_history();
			increment_history_indices();
		}

        buffer[current_terminal][bufferSIZE[current_terminal]] = '\n'; // Place line terminating character.

        terminal_return();
		clear_keyboard_buffer(); // Clear the buffer.
		return;
	}

	// If the keycode isn't a release, translate to the correct Key val using scancode table
	if (!(key & RELEASE) && (ctrl_down != 1))
	{
		// Reached max buffer size. Do nothing.
		if (bufferSIZE[current_terminal] == MAX_CHARACTERS) {
			return;
		}

		if ((shift_down == 1) && (caps_lock != 1) && (alt_down != 1)) {
			shiftkey = key + SHIFT;
			c = scancode[shiftkey];
		} else if ((shift_down != 1) && (caps_lock == 1) && (alt_down != 1)) {
			c = CAPScode[key];
		} else if ((shift_down == 1) && (caps_lock == 1) && (alt_down != 1)) {
			shiftkey = key + SHIFT;
			c = CAPScode[shiftkey];
		} else if ((shift_down != 1) && (caps_lock != 1) && (alt_down == 1)) {
			shiftkey = key + SHIFT;
			c = altcode[shiftkey];
		} else {
			c = scancode[key];
		}
		// If an invalid key is pressed, do nothing.
		if (c == INVALID_SCANCODE) {
			return;
		}

		uint8_t arg = 0;
		//If Alt+F1 is pressed, switch to terminal 1
		if (c == ALT_F1) {
			terminal_switch(arg);
			return;
			//terminal_open(1);
		}

		//If Alt+F2 is pressed, switch to terminal 2
		if (c == ALT_F2) {
			arg = 1;
			terminal_switch(arg);
			return;
			//terminal_open(2);
		}

		//If Alt+F3 is pressed, switch to terminal 3
		if (c == ALT_F3) {
			arg = 2;
			terminal_switch(arg);
			return;
			//terminal_open(3);
		}

		// Insert into character buffer
		keyboard_insert(c);
	}
	else if (!(key & RELEASE) && (ctrl_down == 1)) {
		// Implementation for CTRL + SHIFT + L
		// if ((key == L) && (shift_down == 1)) {

		// Implementation for CTRL + L
		if (key == L) {
			// Clear display entirely, reset buffer.
			clear_keyboard_buffer(); // Clears buffer.
			terminal_clear();		  // Clears terminal.
			// terminal_return();	// ADDED LATER - more clean terminal clear.
		}
		// if (key == C) {
		// 	next_line();
		// 	cursor_update(); // Update cursor
		//
		// 	send_eoi(IRQ1);
		// 	do_call(SYS_HALT,0,0,0);
		// }
		// if (key == D) {
		// 	printf("term = %d, ", current_terminal);
		// 	printf("pid = %d, ", current_pid);
		// 	printf("proc = %d\n", current_process);
		// }
	}
	return;
}

/* void keyboard_insert(char c)
 * Description: Inserts a character at current index.
 * 				Should only be called by handle_keyboard_input()
 * Inputs:      char c - the character to output
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  prints the current keyboard buffer to the screen.
 */
void keyboard_insert(char c)
{
	// Move right side of keyboard buffer over one.
	int i;
	for (i = bufferSIZE[current_terminal]; i >= bufferIDX[current_terminal]; i--)
	{
		char move = buffer[current_terminal][i];
		buffer[current_terminal][i + 1] = move;
	}

	buffer[current_terminal][bufferIDX[current_terminal]] = c;

	unsigned char * ptr = &buffer[current_terminal][bufferIDX[current_terminal]];
	unsigned int num_chars = bufferSIZE[current_terminal] - bufferIDX[current_terminal] + 1;
	key_flag = 1;
	terminal_write(1, ptr, num_chars);
	key_flag = 0;
	cursor_update_left(num_chars - 1);

	bufferIDX[current_terminal]++;
	bufferSIZE[current_terminal]++;
	return;
}

/*
 * keyboard_backspace
 *   DESCRIPTION: Handles a backspace input as it changes the buffer.
 * 				Should only be called by handle_keyboard_input()
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: The character at bufferIDX is removed from the buffer.
 */
void keyboard_backspace()
{
	// Move right side of keyboard buffer over one.
	int i;
	for (i = bufferIDX[current_terminal] - 1; i < bufferSIZE[current_terminal]; i++)
	{
		char move = buffer[current_terminal][i + 1];
		buffer[current_terminal][i] = move;
	}

	buffer[current_terminal][bufferSIZE[current_terminal]] = '\0';

	unsigned char * ptr = &buffer[current_terminal][bufferIDX[current_terminal] - 1];
	unsigned int num_chars = bufferSIZE[current_terminal] - (bufferIDX[current_terminal]) + 1;
	terminal_backspace();
	terminal_write(1, ptr, num_chars);
	cursor_update_left(num_chars);

	bufferIDX[current_terminal]--;
	bufferSIZE[current_terminal]--;
	return;

}

/* void up_history()
 * Description: Scrolls through history buffer
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  If history is full roll back to 0
 */
void up_history()
{
	int displayed_pid = schedule_stack[current_terminal][schedule_top[current_terminal] - 1];

	// If there is no history to go to, exit.
	if (historySIZE[displayed_pid] < NUM_HISTORY_BUFFERS && historyIDX[displayed_pid] == 0) {
		return;
	}

	// Save current keyboard buffer (at base of history)
	if (historyIDX[displayed_pid] == historyBASE[displayed_pid]) {
		copy_to_history();
	}

	// calculate the next history IDX. If this is hitting the ceiling, do nothing.
	// i is the next history IDX.
	int i = historyIDX[displayed_pid] - 1;
	if (i < 0) i = NUM_HISTORY_BUFFERS - 1;

	if (i == historyBASE[displayed_pid]) {
		return;
	} else {
		historyIDX[displayed_pid] = i;
	}

	// If we have a history, clear the existing keyboard buffer.
	reset_terminal_keyboard_input(bufferIDX[current_terminal], bufferSIZE[current_terminal]);
	clear_keyboard_buffer();

	// Copy the history buffer into the keyboard buffer and print to terminal.
	for (i = 0; i < MAX_BUFFER_SIZE; i++)
	{
		char c = history[displayed_pid][historyIDX[displayed_pid]][i];
		if (c == '\0' || c == '\n') {
			break;
		}
		buffer[current_terminal][i] = c;
		bufferIDX[current_terminal]++;
		bufferSIZE[current_terminal]++;
	}
	unsigned char * ptr = &buffer[current_terminal][0];
	unsigned int num_chars = bufferSIZE[current_terminal];
	terminal_write(1, ptr, num_chars);

	return;
}

/* void down_history()
 * Description: Scrolls through history buffer
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  If history is full roll back to 0
 */
void down_history()
{
	int displayed_pid = schedule_stack[current_terminal][schedule_top[current_terminal] - 1];

	// If there is no history to go to, exit.
	if (historySIZE[displayed_pid] < NUM_HISTORY_BUFFERS
		&& historyIDX[displayed_pid] == historyBASE[displayed_pid]) {
		return;
	}

	// Save current keyboard buffer (at base of history)
	// if (historyIDX[current_terminal] == historyBASE[current_terminal] && bufferSIZE[current_terminal] != 0) {
	// 	copy_to_history();
	// }

	// calculate the next history IDX. If this is hitting the ceiling, do nothing.
	if (historyIDX[displayed_pid] == historyBASE[displayed_pid]) {
		return;
	} else {
		historyIDX[displayed_pid] = ((historyIDX[displayed_pid] + 1) % NUM_HISTORY_BUFFERS);
	}

	// If we have a history, clear the existing keyboard buffer.
	reset_terminal_keyboard_input(bufferIDX[current_terminal], bufferSIZE[current_terminal]);
	clear_keyboard_buffer();

	// Copy the history buffer into the keyboard buffer and print to terminal.
	int i;
	for (i = 0; i < MAX_BUFFER_SIZE; i++)
	{
		char c = history[displayed_pid][historyIDX[displayed_pid]][i];
		if (c == '\0' || c == '\n') {
			break;
		}
		buffer[current_terminal][i] = c;
		bufferIDX[current_terminal]++;
		bufferSIZE[current_terminal]++;
	}
	unsigned char * ptr = &buffer[current_terminal][0];
	unsigned int num_chars = bufferSIZE[current_terminal];
	terminal_write(1, ptr, num_chars);

	return;
}

/* void clear_all_keyboard_buffers()
 * Description: Initializes/clears all the keyboard buffers.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  All keyboard buffers are reinitialized.
 */
void clear_all_keyboard_buffers()
{
	int i, j, k = 0;

	// Clear character buffers
	for (j = 0; j < NUM_TERMINALS; j++)
	{
		bufferIDX[j] = 0;
		bufferSIZE[j] = 0;
		for (i = 0; i < MAX_BUFFER_SIZE; i++)
		{
			buffer[j][i] = '\0';
		}
	}

	for (j = 0; j < TOTAL_PROCESSES; j++)
	{
		historyIDX[j] = 0;
		historyBASE[j] = 0;
		historySIZE[j] = 0;
		for (k = 0; k < NUM_HISTORY_BUFFERS; k++)
		{
			for (i = 0; i < MAX_BUFFER_SIZE; i++)
			{
				history[j][k][i] = '\0';
			}
		}
	}
}

/*
 * clear_keyboard_buffer
 *   DESCRIPTION: Clears the current keyboard buffer.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Current buffer is cleared.
 */
void clear_keyboard_buffer()
{
	// clear buffer and buffer index.
	int i;
	for (i = 0; i < MAX_BUFFER_SIZE; i++)
	{
        buffer[current_terminal][i] = '\0';
    }

    bufferIDX[current_terminal] = 0;
    bufferSIZE[current_terminal] = 0;
}

/*
 * clear_history_buffer()
 *   DESCRIPTION: Clears all history data with given process id.
 *   INPUTS: int pid - the process id to clear the history of.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: All history data for given pid is cleared.
 */
void clear_history_buffer(int pid)
{
	// clear history buffer and buffer index.
	int i, j = 0;
	for (i = 0; i < NUM_HISTORY_BUFFERS; i++)
	{
		for (j = 0; j < MAX_BUFFER_SIZE; j++) {
			history[pid][i][j] = '\0';
		}
    }

	historyIDX[pid] = 0;
	historyBASE[pid] = 0;
	historySIZE[pid] = 0;
}

/*
 * keyboard_left_arrow
 *   DESCRIPTION: Handles a backspace input as it changes the buffer.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: The character at bufferIDX is removed from the buffer.
 */
void keyboard_left_arrow()
{
    if (bufferIDX[current_terminal] == 0) {
        return;
    } else {
        bufferIDX[current_terminal]--;
		cursor_update_left(1);
    }
}

/*
 * keyboard_right_arrow
 *   DESCRIPTION: Handles a backspace input as it changes the buffer.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: The character at bufferIDX is removed from the buffer.
 */
void keyboard_right_arrow()
{
	if (bufferIDX[current_terminal] == bufferSIZE[current_terminal]) {
        return;
    } else {
        bufferIDX[current_terminal]++;
		cursor_update_right(1);
    }
}
