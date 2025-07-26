#include "kernel.h"
#include "idt.h"
#include <stdbool.h>
extern void problem();
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
	idt_init();
	print("hello world\n");
	problem();
	return;
}
