#include "kernel.h"
#include "./idt/idt.h"
#include <stdbool.h>
#include "./io/io.h"
#include "./pit/pit.h"
#include "./heap/kheap.h"
#include "./heap/heap_cream.h"
#include "./paging/paging.h"
#include "./disk/disk.h"
#include "./time/time.h"
#include "./disk/disk_stream.h"
#include "./file_syst/partitions.h"

extern void problem21();
extern void enable_interrupts();
uint16_t* vedio=(uint16_t*)0x000b8000;
uint16_t cursor_x=0;
uint16_t cursor_y=0;
uint8_t G_BOOT_DRIVE=0;	//for setting values for disk
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

static void scroll1()
{
	for(int i=0;i<24;i++)
	{
		for(int j=0;j<79;j++)
			vedio[(i*80)+j]=vedio[((i+1)*80) +j];
	}
	for(int i=0;i<79;i++)
		vedio[(24*79)+i]=0;
	
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
				if(cursor_y<24)
						cursor_y++;
					else
					{
						scroll1();
						//cursor_y
					}
				cursor_x=0;
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
					if(cursor_y<24)
						cursor_y++;
					else
					{
						scroll1();
						//cursor_y
					}
					cursor_x=0;
				}
				break;
		}
	}
}



void print_hex(uint8_t b)
{
	char *hex="0123456789abcdef";
	char str[3]={hex[(b>>4)&0xf], hex[b & 0xf],'\0'};
	print(str);
}

void print_16(uint16_t val)
{
	print_hex((uint8_t)((val>>8)&0xff));
	print_hex((uint8_t)(val&0xff));
}

void print_32(uint32_t val)
{
	print_16((uint16_t)((val>>16) &0xffff));
	print_16((uint16_t)(val&0xffff));
}

void print_64(uint64_t val)
{
	print_32((uint32_t)((val>>32)&0xffffffff));
	print_32((uint32_t)(val & 0xffffffff));
}
void kernel_main(uint8_t boot_drive)
{
	G_BOOT_DRIVE =boot_drive;
	clear_screen();
	
	print("\t\tGENESIS-32 \nkernel loaded successfully.\n");
	
	kheap_init();
	
	print("kernel heap setup initalized....\n");
	
	
	idt_init();
	print("interrupt table loaded .....\n");
	
	pit_init((uint32_t) 100);
	print("set pit to a safe 100hz");
	
	
	dir_table_address directory_table=create_32_dir_table(PAGING_PRESENT|READ_AND_WRITE);
	print("directory table for 4gb set....\n");
	set_dir_table(directory_table);
	print("directory table loaded......\n");
	
	
	enable_paging();
	print("paging enabled...\n");

	help_me_mary_mother_of_god();
	
	
	print("searching for disks....\n");
	disk_search_and_init();
	
	print("printing info obtained\n\n");
	disk_debug_print();
	
	print("searching for partitions..\n");
	scan_part_all_disks();
	
	partition_debug();
	
	
	enable_interrupts();
	print("interrupts enabled....\n");
	
	
	
	
	
	return;
}
