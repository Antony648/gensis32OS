#include "osconfig.h"
#include "idt.h"
#include "essentials.h"
#include "kernel.h"
extern void load_idtr(void* ldtr_addr);
extern void sr0();
struct idt_desc idt_table[TOTAL_INTERRUPTS];
struct idtr idtr_descriptor;
void divide_zero()
{
	print("divide by zero error\n");
}
//function to setup a standard idt entry of type 32IKC31
void setup_idt32IKC31(uint32_t interrupt_no, void* address)
{
	 
	idt_table[interrupt_no].offset_l =(uint32_t)address & 0x0000ffff;  //we cancel upper 16btis
	idt_table[interrupt_no].offset_h=((uint32_t)address )>>16; // we zeroed lower 16bits and rightshift 16
	idt_table[interrupt_no].selector=KERNEL_CODE_SELECTOR;
	idt_table[interrupt_no].attr=_32INTERRUPT_KC_3_1;
	idt_table[interrupt_no].zero=0x00;
}
void idt_init()
{
	print("calling idt_init\n");
    memset(idt_table,0,sizeof(idt_table));	//clear descriptor table
    
    idtr_descriptor.len=sizeof(idt_table)-1; 
    idtr_descriptor.addr=(uint32_t)idt_table;
		//set values for  idtr_descriptor
	setup_idt32IKC31(0x20,sr0);
		//load address for ISR 0x00
    load_idtr(&idtr_descriptor);
		//load value of idt table
}

