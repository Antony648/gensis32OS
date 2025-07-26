#ifndef KERNEL_H
#define KERNEL_H
#include <stdint.h>
#define VGA_HEIGHT 25
#define VGA_WIDTH  80

void print(char * str);
void print_string_x_y(char*str,uint8_t x,uint8_t y,char color);
void print_x_y(char letter,uint8_t x,uint8_t y,char color);
void print_string(char* str);
void clear_screen();
void kernel_main();


#endif	
