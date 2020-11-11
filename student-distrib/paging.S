# paging.S - Sets up paging
# vim:ts=4 noexpandtab

#define ASM     1
#define CR_FOUR_FOUR $0x00000010
#define CR_FOUR_FIVE $0xffffffdf
#define CR_ZERO_LAST $0x80000000


.text
.globl init_control_registers_paging
.globl set_control_registers_paging
.globl flush_tlb

/* void init_control_registers_paging()
 * Description: Enables Paging, stores page directory in cr3, enables 4mb pages
 * Inputs:      page directory
 * Outputs:     NONE
 * Return Value: NONE
 * Side Effects:  NONE*/
init_control_registers_paging:
    pushl %ebp
    movl %esp, %ebp
    movl %cr4, %eax
    orl CR_FOUR_FOUR, %eax
    andl CR_FOUR_FIVE, %eax
    movl %eax, %cr4             # enables 4mB paging in cr4
    movl 8(%esp), %eax
    movl %eax, %cr3             # puts paging directory into cr3
    movl %cr0, %eax
    orl CR_ZERO_LAST, %eax
    movl %eax, %cr0            # rewrites cr0 to enable paing
    movl %ebp, %esp
    popl %ebp
    ret

set_control_registers_paging:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%esp), %eax
    movl %eax, %cr3             # puts paging directory into cr3
    movl %ebp, %esp
    popl %ebp
    ret

flush_tlb:
    pushl %eax
    movl %cr3, %eax
    movl %eax, %cr3             # puts paging directory into cr3
    popl %eax
    ret
