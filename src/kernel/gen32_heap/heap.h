#ifndef HEAP_H
#define HEAP_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define	 HEAP_START 0X1000000
#define  HEAP_SIZE 0X2000000
#define	 HEAP_HEADER 16
#define  HEAP_MIN_LIMIT 16
struct heap_table_entr
{
	size_t size;						//32bit unsigned int type
	struct heap_table_entr* addr_n;		//32bit address to next block
	bool free;							//8bit unsigned int type
	uint32_t flags;						//flags for later use mainly for padding so struct rounds up to 16bytes
		//flags has 32bits so 32potential flag values will be very useful in protecting process heap
	};
	//overall size of heap_table_entr is 13bytes but struct is rounded to mulitples of 4 so 16
void heap_init();	//should be called by kernel.c just after idt_init()
void * malloc(size_t size);
void free(void* ptr);
#endif
