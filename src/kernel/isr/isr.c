# include "isr.h"
#include "../kernel.h"
#include "../io/io.h"
void divide_zero()
{
	print("divide by zero error");
	}
void int21_handler()
{
	print("test int 0x21");
	//char a=inb(0x60);
	//print(&a);
	}
void int20_handler()
{
	
}
void no_interrupt()
{
	//print("isr not written yet..hehe");
	}
