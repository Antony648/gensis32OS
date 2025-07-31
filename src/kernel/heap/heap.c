 #include "heap.h"
#include "../kernel.h"
#include "../error.h"
#include "../essentials/essetials.h"
#include <stdbool.h>
#include <stdeff.h>
static bool heap_validate_alignment(void *end)	//checks if the heap is aligned to 4096
{
	return ((end%KHEAP_BLOCK_SIZE)==0);
	}
static bool heap_entry_count(struct heap* heap_val,struct heap_table* heap_t)
{
	size_t cal_val=(size_t)((heap_val->end_addr-heap_val->start_addr)/KHEAP_BLOCK_SIZE);
	if (cal_val==heap_t.total)
		return true;
	}
int heap_create(struct heap* heap_val,void* ptr,void* end,struct heap_table* heap_t)
{
	
	if(!heap_validate_alignment(end)|| !heap_validate_alignment(ptr))
		ret -GEN32_INVARG;
		
	heap_val->table=0x00;	
	heap_val->start_addr=ptr;
	heap_val->end_addr=end;
	
	if(!heap_entry_count(heap_val,heap_t))
		ret -GEN32_INVARG;
		
	//set all table entries to 0x00 each table entry is a byte
	memset(heap_t->entries,(int)0x00,(size_t)(heap_t->total*sizeof(heap_block_entry)));
	
	
	return 0;
}
void* heap_malloc(size_t size)
{
	size=(size+0xfff)>>12;	//used to obtain number of pages requried 
	//here instead of creating a d
	}

void* heap_free()
{
	}
