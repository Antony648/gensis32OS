#include "kernel.h"
#include "./idt/idt.h"
#include <stdbool.h>
#include "./io/io.h"
#include "./heap/kheap.h"
#include "./heap/heap_cream.h"
#include "./paging/paging.h"
#include "./disk/disk.h"
#include "./disk/disk_stream.h"

extern void problem21();
extern void enable_interrupts();
uint16_t* vedio=(uint16_t*)0x000b8000;
uint16_t cursor_x=0;
uint16_t cursor_y=0;
uint16_t text_mode_hex_gen(char letter,uint8_t fg,uint8_t bg,bool blink)
{
	uint16_t rtn_val=0x00;	//rtn_val is 0x00000000
	rtn_val|=blink;
	rtn_val=rtn_val<<3;
	bg&=0x0007;	//only last 3 bits value will be retained to ensure that if 16bit value is passed it is converted to a 3bit one
	rtn_val|=bg;
	rtn_val=rtn_val<<4;
	fg&=0x000f;
	rtn_val|=fg;
	rtn_val=rtn_val<<8;
	rtn_val|=letter;
	return rtn_val;
}
uint16_t simp_text_hex_gen(char letter,char color)
{
	return (color<<8 | letter);
}



void clear_screen()
{	int limit=VGA_HEIGHT*VGA_WIDTH;	
	for(int i=0;i<limit;i++)
		vedio[i]=0x0020;
	cursor_x=0;cursor_y=0;
}
void print_string(char* str)
{
	for(int i=0;str[i]!='\0';i++)
		vedio[i]=text_mode_hex_gen(str[i],3,0,0);
}


void print_x_y(char letter,uint8_t x,uint8_t y,char color)
{
	vedio[(y*VGA_WIDTH)+x]=simp_text_hex_gen(letter,color);
}
void print_string_x_y(char*str,uint8_t x,uint8_t y,char color)
{
	uint16_t index= VGA_WIDTH*y +x;
	for(int i=0;str[i]!='\0';i++,index++)
	{
		vedio[index]= simp_text_hex_gen(str[i],color);
	}
}
void print(char * str)
{
	for(int i=0;str[i]!='\0';i++)
	{
		switch(str[i])
		{
			case '\n':
				cursor_y++;cursor_x=0;
				break;
			case '\t':
				cursor_x+=5;
				break;
			default:
				vedio[(cursor_y * VGA_WIDTH)+cursor_x]=simp_text_hex_gen(str[i],15);
			/*	cursor_x++;
				cursor_y+=(cursor_x/80)
				cursor_x=(cursor_x)%80;*/
				cursor_x++;
				if(cursor_x>79)
				{
					cursor_y++;cursor_x=0;
				}
				break;
		}
	}
}
void kernel_main()
{
	clear_screen();
	print("\t\tGENESIS-32 \nkernel loaded successfully.\n");
	kheap_init();
	
	print("kernel heap setup initalized....\n");
	
	disk_search_and_init();
	print("searching for disks....");
	
	idt_init();
	print("interrupt table loaded .....\n");
	
	
	
	dir_table_address directory_table=create_32_dir_table(PAGING_PRESENT|READ_AND_WRITE);
	print("directory table for 4gb set....\n");
	set_dir_table(directory_table);
	print("directory table loaded......\n");
	
	
	enable_paging();
	print("paging enabled...\n");
	
	
	
	enable_interrupts();
	print("interrupts enabled....\n");
	
	//heap cream tests
	uintptr_t karray[5];
	heap_cream_init(karray);
	//karray is zeroed
	void* p1=heap_cream_malloc(karray,32);
	//should create an entry in karray pointing to first allocated pages,and p1 should be that address +0x80
	void* p2=heap_cream_malloc(karray,32);
	//should not create further entries in karray, should return p1+0x20 if working fine
	void* p3=heap_cream_malloc(karray,96);
	//shold not create further entries in karray, shoule return p1+0x40 if working fine
	//the table inside karray should be 0x41 0x41 0xc1 0x81 0x01
	heap_cream_free(karray,p2);
	//should unset the second table entry of karray[0];
	void* p5=heap_cream_malloc(karray,107);
	//should give p5=p3+0x60 and set 
	void* p4=heap_cream_malloc(karray,3899);
	//should add one more entry to karray, and allocate space in that page
	//break here to know if pages are resused and freed.
	void* p6=heap_cream_malloc(karray,45);
	//should allocate spece in first page array[0]
	heap_cream_free(karray,p3);
	//should free from first page , karray still should contains 2 values
	heap_cream_free(karray,p4); 
	//should remove second element from the karray
	heap_cream_free(karray,p1);
	//should change table in first block
	heap_cream_free(karray,p2);
	//should throw error as we double free it, 
	
	
	heap_cream_free(karray,p5);
	//should free
	heap_cream_free(karray,p6);
	//should completely free the karray
	
	
	
	void* p7=heap_cream_malloc(karray,4000);
	//should give null, and message to use kzalloc
	
	
	
	heap_cream_free(karray,p7);
	//should give false, and message that we tried to free null
	
	
	
	heap_cream_destroy(karray);
	void* p8=kzalloc(1);
	void* p9=kzalloc(1);
	//check the values of p8 and p9 and value of  karray[0] and karray[1]. to know if its freed
	kfree(p9);
	kfree(p8);
	return;
}
