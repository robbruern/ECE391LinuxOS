 /* interrupts.c - Setting up the idt
  * vim:ts=4 noexpandtab
  */

#include "interrupts.h"
#include "x86_desc.h"
#include "i8259.h"
#include "exception-handlers.h"
#include "filesystem_driver.h"
#include "syscalls_wrapper.h"
#include "rtc_wrapper.h"
#include "rtc_driver.h"
#include "keyboard.h"
#include "keyboard_wrapper.h"

#include "schedule_wrapper.h"
#include "lib.h"
#include "process_control.h"

extern node_block_t * node_list;
extern void init_control_registers_paging(unsigned int * page);
extern void set_control_registers_paging(unsigned int * page);
extern void flush_tlb();

void timer_handler();

int sanity_check;
int last_went[3];
/*flag for rtc interrupt handler*/
#define TRAP_GATE 0xF
#define INTR_GATE 0xE

 int rtc_interrupt_flag[3];

/*Local function declaration*/
static int color_flag;
idt_desc_t create_idt_entry(int seg_select, int dpl, int present);

/*
 * initialize_IDT
 *   DESCRIPTION: Creates the IDT, containing the exceptions and interrupts,
 *                  and the empty handler for the system calls.
 *                  And then loads the IDT.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void initialize_IDT()
{

	/* Init PIC*/
	i8259_init();

	/* Masks all interrupts but PS/2 keyboard on the PIC*/
	outb(0xFD, MASTER_8259_PORT + 1);
  	outb(0xFF, SLAVE_8259_PORT + 1);
 	/*Exceptions: 0x00-0x1F*/

    /*Create idt entry 0, for dividing by zero*/
	idt[0x00] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x00], exception_div_zero);

    /*Create idt entry 1, for debug*/
	idt[0x01]  = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x01], exception_debug);

    /*Create idt entry 2, for nonmaskable interrupts*/
    idt[0x02] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x02], exception_nonmaskable_interrupt);

    /*Create idt entry 3, for breakpoints*/
	idt[0x03] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x03], exception_breakpoint);

    /*Create idt entry 4, for overflow*/
	idt[0x04]  = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x04], exception_overflow);

    /*Create idt entry 5, for bound range exceeded*/
	idt[0x05]  = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x05], exception_bound_range_exceeded);

    /*Create idt entry 6, for invalid opcode*/
	idt[0x06] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x06], exception_invalid_opcode);

    /*Create idt entry 7, for device not available*/
	idt[0x07] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x07], exception_device_not_available);

    /*Create idt entry 8, for double faults*/
	idt[0x08]  = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x08], exception_double_fault);

    /*Create idt entry 9, for coprocessor segment overrun*/
	 idt[0x09] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x09], exception_coprocessor_segment_overrun);

    /*Create idt entry 10, for invalid TSS*/
	idt[0x0A] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x0A], exception_invalid_TSS);

    /*Create idt entry 11, for segment not present*/
	idt[0x0B]= create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x0B], exception_segment_not_present);

    /*Create idt entry 12, for stack segment faults*/
	idt[0x0C]  = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x0C], exception_stack_segment_fault);

    /*Create idt entry 13, for general protection faults*/
	idt[0x0D] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x0D], exception_general_protection_fault);

    /*Create idt entry 14 for page faults*/
	idt[0x0E] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x0E], exception_page_fault);

    /*Create idt entry 15 for reserved*/
	idt[0x0F] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x0F], exception_reserved);

    /*Create idt entry 16 for x87 floating point exceptions*/
	idt[0x10] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x10], exception_x87_floating_point);

    /*Create idt entry 17 for alignment check*/
	idt[0x11] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x11], exception_alignment_check);

    /*Create idt entry 18 for machine check*/
	idt[0x12] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x12], exception_machine_check);

    /*Create idt entry 19 for SIMD floating point exceptions*/
	idt[0x13] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x13], exception_SIMD_floating_point);

    /*Create idt entry 20 for virtualization exceptions*/
	idt[0x14] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x14], exception_virtualization);

    /*Create idt entry 21 for reserved*/
	idt[0x15] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x15], exception_reserved);

    /*Create idt entry 22 for reserved*/
	idt[0x16] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x16], exception_reserved);

    /*Create idt entry 23 for reserved*/
	idt[0x17] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x17], exception_reserved);

    /*Create idt entry 24 for reserved*/
	idt[0x18] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x18], exception_reserved);

    /*Create idt entry 25 for reserved*/
	idt[0x19] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x19], exception_reserved);

    /*Create idt entry 26 for reserved*/
	idt[0x1A] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x1A], exception_reserved);

    /*Create idt entry 27 for reserved*/
	idt[0x1B] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x1B], exception_reserved);

    /*Create idt entry 28 for reserved*/
	idt[0x1C] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x1C], exception_reserved);

    /*Create idt entry 29 for reserved*/
	idt[0x1D] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x1D], exception_reserved);

    /*Create idt entry 30 for security exception*/
	idt[0x1E] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x1E], exception_security);

    /*Create idt entry 31 for reserved*/
    idt[0x1F] = create_idt_entry(KERNEL_CS, DPL_KERNEL, PRESENT_MASK);
    SET_IDT_ENTRY(idt[0x1F], exception_reserved);

    /*Create idt entry 33 for the schedule wrapper*/
	idt[0x20] = create_idt_entry(KERNEL_CS, DPL_USER, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x20], schedule_wrapper);

	/*Create idt entry 34 for the keyboard wrapper*/
	idt[0x21] = create_idt_entry(KERNEL_CS, DPL_USER, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x21], keyboard_wrapper);

    /*Create idt entry 41 for the keyboard wrapper*/
	idt[0x28] = create_idt_entry(KERNEL_CS, DPL_USER, PRESENT_MASK);
	SET_IDT_ENTRY(idt[0x28], rtc_wrapper);

    /*Create idt entry 128 for the keyboard wrapper*/
	idt[0x80] = create_idt_entry(KERNEL_CS, DPL_USER, PRESENT_MASK);
	//SET_IDT_ENTRY(idt[0x80], syscall_wrapper);
	SET_IDT_ENTRY(idt[0x80], system_call);

	/*Load the IDT*/
	lidt(idt_desc_ptr);

    /*Set the color flag to zero*/
	color_flag = 0;

	// Enable the cursor.
	cursor_disable();
	cursor_enable();
	cursor_update();

	return;
}

/* void PIT_init()
 * Description: Enables IRQ0 interrupts to be sent from the
 * 							PIT chip.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Enables IRQ1 signal from the PIC and
 *								then initializes the PIT to run on 10ms intervals.
 */
void PIT_init()
{
	int i = 0;
	int divisor = 1193180/100;// 100; // Calculate our divisor; 100 corresponds to desired frequency in Hz
	outb(0x36, COMMAND_REGISTER);					//Sets the command byte 0x36
	outb(divisor & 0xFF, CHANNEL_0_DATA_REGISTER);				 //Sets the low byte
	outb(divisor >> 8, CHANNEL_0_DATA_REGISTER);  // Sets the high byte
	timer_ticks = 0;
	for (i = 0; i < 3; i++) {
		schedule_info_arr[i].s_ebp = 0;
		schedule_info_arr[i].s_esp = 0;
		strand_type_lock = 0;
	}
	current_process = 0;
	sanity_check = 0;
	terminal_request = -1;
	schedule_tick = 0;
	enable_irq(IRQ0);

	return;
}

/* void timer_handler()
 * Description: Is the PIT chip interrupt handler that handles scheduling.
 * 							Given the 10ms timeslice, the function will do a context switch
 *              when required.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Saves context of current task and switches to next task
 *								in round robin interval.
 */
void timer_handler()
{
	send_eoi(IRQ0);
	cli();

	flush_tlb();


	if (schedule_top[current_process] != 0)
	{
		schedule_tss[current_process][schedule_top[current_process] - 1] = tss;
		schedule_esp[current_process][schedule_top[current_process] - 1] = last_esp;
		schedule_ebp[current_process][schedule_top[current_process] - 1] = last_ebp;
		sanity_check = last_esp;
	}

	int next_process;
	if (terminal_request == -1) {
		// Increment current_process loop counter.
		next_process = (current_process + 1) % NUM_TERMINALS;
		while (schedule_top[next_process] == 0)  {
			next_process = (next_process + 1) % NUM_TERMINALS;
		}
	} else if (terminal_request == 0) {
		next_process = 0;
	} else if (terminal_request == 1) {
		next_process = 1;
	} else if (terminal_request == 2) {
		next_process = 2;
	}

	current_process = next_process;
	terminal_request = -1;

	if (schedule_top[current_process] != 0)
	{
		if (schedule_ebp[current_process][schedule_top[current_process] - 1] != 0)
		{
			current_pid = schedule_stack[current_process][schedule_top[current_process] - 1];
			last_esp = schedule_esp[current_process][schedule_top[current_process] - 1];
			last_ebp = schedule_ebp[current_process][schedule_top[current_process] - 1];
			tss = schedule_tss[current_process][schedule_top[current_process] - 1];

	        process_pages[VIRTUAL_PROGRAM] = (M_4 * (current_pid + 1)) | PAGE_ARGS;

			if (current_process != current_terminal) {
				process_tables[VID_IDX] = (VIDEO_1 + (0x1000 * current_process)) | USER_MASK | READWRITE_MASK | PRESENT_MASK;
				video_mem = (int8_t *)(VIDEO_1 + (0x1000 * current_process));
			} else {
				process_tables[VID_IDX] = VIDEO | USER_MASK | READWRITE_MASK | PRESENT_MASK;
				video_mem = (int8_t *)VIDMAP;
			}

			int addr = (int)&process_tables[VID_IDX];
			process_pages[VID_IDX] = ((addr >> 12) << 12) | 0x7;

			flush_tlb();

			tss.esp0 = KERNEL_ADDR + (M_4 - 0xF) - (2 * K_4 * current_pid);
		}
	}


	if (schedule_tick == 0) {
		schedule_tick = 1;
	} else {
		schedule_tick = 0;
	}

	return;
}

/* void RTC_init()
 * Description: Enables IRQ8 interrupts to be sent from the
 * 							RTC chip. Then unmasks the IRQ2 and IRQ8,
 *							to receive the interrupts through the PIC
 *							key to the screen.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Enables IRQ8 signal from the PIC and
 *								then unmasks the IRQ2 and IRQ8 on the PIC.
 */
void RTC_init()
{
	outb(0x8B, CMOS_REG);					//selects reg B, and disables NMIs
	char prev;
	prev = inb(RTC_REG);
	outb(0x8B, CMOS_REG);				 //set the index again to reg B
	outb(prev | 0x40, RTC_REG);  //enable IRQ8 interrupt flag
  int flags = 0;
	enable_irq(IRQ2);
	enable_irq(IRQ8);
	rtc_optable();
	rtc_running[0] = 2;
	rtc_running[1] = 2;
	rtc_running[2] = 2;
  rtc_virtual_interupt[0] = 0;
  rtc_virtual_interupt[1] = 0;
  rtc_virtual_interupt[2] = 0;

  // Need to enable interrupts
  restore_flags(flags);

	return;
}

/* void keyboard_init()
 * Description: Enables the IRQ1 pin on the PIC for the keyboard
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  IRQ1 PIC on the master PIC is unmasked
 */
void keyboard_init()
{
	keyboard_open('\0');
	enable_irq(IRQ1);
}

/* void paging_init()
 * Description: Builds a 4kb page directory and a 4kb page table to copy the
 * kernel and video memory into virtual memory
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: pointer to dest
 * Side Effects:  NONE*/
void paging_init()
{
	int i; /*used for loop iterations*/

	/*intializes paging and don't need to do anything for 8Mb to 4Gb*/
	for (i = 0; i < PAGE_SIZE; i++)
	{
		/*page_directory[i]*/process_pages/*[0]*/[i] = 0x00000002;
	}

	/*intializes first table in mem*/
	for (i = 0; i < PAGE_SIZE; i++)
	{
		/*first_table[i]*/process_pages/*[0]*/[i] =  0x00000002;
	}

	/*maps the kernel, sets it to present*/
	process_pages/*[0]*/[1] = KERNEL;

	/*maps video memory, sets it to present*/
	process_tables/*[0]*/[VIDEOMEM] = VIDEO  | PRESENT_MASK;

	/*sets up first table in the directory*/
  	process_pages/*[0]*/[0] = ((unsigned int)process_tables/*[0]*/)| PRESENT_MASK | READWRITE_MASK;
	init_control_registers_paging(process_pages/*[0]*/);

	return;
}

/*
 * idt_desc_t
 *   DESCRIPTION: returns an idt entry, based on the passed
 *    in info and sets all the reserved field to zero.
 *    ie: segment selector, dpl and present
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: idt_desc_t, containing the idt entry info
 *   SIDE EFFECTS: none
 */
idt_desc_t create_idt_entry(int seg_select, int dpl, int present)
{
	idt_desc_t temp;

	/*Populate the idt with the parameters*/
	temp.seg_selector = seg_select;
	temp.present = present;
	temp.dpl = dpl;

	/*Fill the preset values*/
	temp.size = 1;
	temp.reserved0 = 0;
	temp.reserved1 = 1;
	temp.reserved2 = 1;
	temp.reserved3 = 0;
	temp.reserved4 = 0;

	return temp;
}

/* void handle_keyboard()
 * Description: Called when keyboard interrupt is triggered
 * 							Reads from the keyboard port, and translates
 *							the key using a scancode table. Then puts the
 *							key to the screen.
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Pressed key is put into the character stream
 */
void handle_keyboard()
{
	int flags = 0;

	cli_and_save(flags);

	char * video_past = video_mem;
	video_mem = (int8_t *)VIDEO;

	unsigned short key;
	key = 0;
	key = inb(PS2PORT);
	handle_keyboard_input(key);

	video_mem = video_past;

	send_eoi(IRQ1);
	restore_flags(flags);
	return;
}

/*
 * handle_RTC
 *   DESCRIPTION: Handles the RTC interrupt by flashing the screen red
 *                and sends the eoi
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void handle_RTC()
{
    /*Local var to save flags*/
	int flags = 0;
    //Mask interrupts and save flags
	cli_and_save(flags);
  rtc_virtual_interupt[0] = rtc_virtual_interupt[0] + 1;
  rtc_virtual_interupt[1] = rtc_virtual_interupt[1] + 1;
  rtc_virtual_interupt[2] = rtc_virtual_interupt[2] + 1;

    /*Send eoi to interrupt port 8*/

	send_eoi(IRQ8);
	outb(0x0C, CMOS_REG);
	inb(RTC_REG);

    //UnMask interrupts and restore flags
	restore_flags(flags);
	return;
}
