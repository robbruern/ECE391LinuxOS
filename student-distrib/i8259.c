/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"
#include "communication.h"
/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* void enable_irq()
 * Description: Initializes the PIC in order to enable interrupts
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Sends the three initialization words to the PIC
 */
void i8259_init(void) {

	/*Initialize both the master and slave port*/
	outb(ICW1, MASTER_8259_PORT);
	outb( ICW1, SLAVE_8259_PORT);

	/*Send vector offset to the PICs*/

	outb( ICW2_MASTER, MASTER_8259_PORT + 1);
	outb( ICW2_SLAVE, SLAVE_8259_PORT + 1);


	/*Send the master/slave configuration to the PICs*/

	outb( ICW3_MASTER, MASTER_8259_PORT + 1);
	outb( ICW3_SLAVE, SLAVE_8259_PORT + 1);

	/*Send additional info about the enviroment to the PICs*/

	outb( ICW4, MASTER_8259_PORT + 1);
	outb( ICW4, SLAVE_8259_PORT + 1);
  return;
}
/* void enable_irq()
 * Description: Unmasks the desired IRQ on the PIC
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Desired IRQ on the PIC can now take interrupts
 */
void enable_irq(uint32_t irq_num) {
	uint16_t port;
  uint8_t val;
  if(irq_num < 8){
    port = MASTER_8259_PORT + 1;
    val = inb(port) & ~(1 << irq_num);
    master_mask = val;
  }
  else{
    port = SLAVE_8259_PORT + 1;
    irq_num -= 8;
    val = inb(port) & ~(1 << irq_num);
    slave_mask = val;
  }
  outb(val, port);
  return;
}
/* void disable_irq()
 * Description: Masks the desired IRQ on the PIC
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Desired IRQ on PIC can no longer accept interrupts
 */
void disable_irq(uint32_t irq_num) {
	uint16_t port;
  uint8_t val;
  if(irq_num < 8){
    port = MASTER_8259_PORT + 1;
    val = inb(port) | (1 << irq_num);
    master_mask = val;
  }
  else{
    port = SLAVE_8259_PORT + 1;
    irq_num -= 8;
    val = inb(port) | (1 << irq_num);
    slave_mask = val;
  }
  outb(val, port);
  return;
}
/* void send_eoi()
 * Description: Sends the EOI signal to desired IRQ
 * Inputs:      NONE
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  Will reenable interrupts on the PIC
 */
void send_eoi(uint32_t irq_num) {
  if(irq_num < 8){
    outb(EOI | (irq_num& 0x07), MASTER_8259_PORT);
  }
  else{
	  irq_num -= 8;
    outb(EOI | 0x02, MASTER_8259_PORT);
    outb(EOI | irq_num, SLAVE_8259_PORT);

  }
  return;
}
