#include "../osconfig.h"
#include "idt.h"
#include "../essentials/essentials.h"
#include "../kernel.h"
extern void load_idtr(void* ldtr_addr);
extern void sr0();
extern void sr21();
extern void isr_x();
extern void sr20();
struct idt_desc idt_table[TOTAL_INTERRUPTS];
struct idtr idtr_descriptor;

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
	print("starting idt load....\n");
    memset(idt_table,0,sizeof(idt_table));	//clear descriptor table
    
    idtr_descriptor.len=sizeof(idt_table)-1; 
    idtr_descriptor.addr=(uint32_t)idt_table;
    //set values for  idtr_descriptor
    
    for(int i=0;i<TOTAL_INTERRUPTS;i++)
		setup_idt32IKC31(i,isr_x);
    
	setup_idt32IKC31(0x00,sr0);  	//load address for ISR 0x00
	setup_idt32IKC31(0x21,sr21);
	//setup_idt32IKC31(0x20,sr20);
    load_idtr(&idtr_descriptor);	//load value of idt table
    

    
	print("idt loading successfull\n");
}

