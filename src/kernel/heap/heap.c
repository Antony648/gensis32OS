 #include "heap.h"
#include "../kernel.h"
#include "../error.h"
#include "../essentials/essentials.h"
#include <stdbool.h>
#include <stddef.h>
static bool heap_validate_alignment(void *end)	//checks if the heap is aligned to 4096
{
	return (((int)end % KHEAP_BLOCK_SIZE)==0);
}
static bool heap_entry_count(struct heap* heap_val,struct heap_table* heap_t)
{
	size_t cal_val=(size_t)((heap_val->end_addr-heap_val->start_addr)/KHEAP_BLOCK_SIZE);
	if (cal_val==heap_t->total)
		return true;
	return false;
}
int heap_create(struct heap* heap_val,void* ptr,void* end,struct heap_table* heap_t)
{
	
	if(!heap_validate_alignment(end)|| !heap_validate_alignment(ptr))
		return -GEN32_INVARG;
		
	heap_val->table=0x00;	
	heap_val->start_addr=ptr;
	heap_val->end_addr=end;
	
	if(!heap_entry_count(heap_val,heap_t))
		return -GEN32_INVARG;
		
	//set all table entries to 0x00 each table entry is a byte
	memset(heap_t->entries,(int)0x00,(size_t)(heap_t->total*sizeof(heap_block_entry)));
	
	
	return 0;
}
heap_block_entry* heap_get_start_block(struct heap* heap_val, uint32_t block_count)
{
	heap_block_entry* i;
	heap_block_entry* max=heap_val->table->entries+heap_val->table->total;
	heap_block_entry* rtn_val=NULL;
	int count=0;
	for(i=heap_val->table->entries;i<max;i++)
	{
		if(*i ==0x00)
		{
			rtn_val=i;
			count++;
			i++;
			while( count<block_count && i<max && *i ==0x00 )
			{
				count++;i++;
			}
			if(count==block_count)
				return rtn_val;
			rtn_val=NULL;
			count=0;
		}
			
	}
	return rtn_val;
}
void* heap_malloc(struct heap* heap_val,size_t size)
{
	//size=(size+0xfff)>>12;	used to obtain number of pages requried 
	//this will give the exact answer for 4096 but for flexibility we should use macros
	uint32_t block_count=(size+KHEAP_BLOCK_SIZE-1)>>KHEAP_BLOCK_SHIFT;
	//iterate through table till we find neccessary amount of blocks in series
	heap_block_entry* address=heap_get_start_block( heap_val,  block_count);
	if(address==NULL)
		return NULL;
	//mark taken blocks as taken
	if(block_count==1)
		*address=HEAP_TABLE_BLOCK_SINGLE;
	else
	{
		*address=HEAP_TABLE_BLOCK_START;
		heap_block_entry* temp;
		for(temp=address+1;temp<=address+block_count-2;temp++)
			*temp=HEAP_TABLE_BLOCK_MID;
		*(address+block_count-1)=HEAP_TABLE_BLOCK_END;
		
		}
	//convert entry in table to absolute address
	uintptr_t base=(uintptr_t)(address - heap_val->table->entries);
	void* abs_addr=(void*)((uintptr_t)heap_val->start_addr+(base*KHEAP_BLOCK_SIZE));
	return  abs_addr;
	}

void heap_free(struct heap* heap_val, void* abs_addr)
{
	uint32_t i=(uint32_t)(abs_addr-heap_val->start_addr)/KHEAP_BLOCK_SIZE;
	heap_block_entry*ptr= (heap_block_entry*)((uintptr_t)heap_val->table->entries+i);
	if(*ptr ==HEAP_TABLE_BLOCK_SINGLE)	//only a single block
	{
		*ptr=0x00;
		return;
	}
	//it is a multiblock , that starts with 0xc1 and ends with 0x01 may/may not have 0x81 in between
	
	else if(*ptr ==HEAP_TABLE_BLOCK_START)
	{
		heap_block_entry* max=heap_val->table->entries+heap_val->table->total;
		while(*ptr  != HEAP_TABLE_BLOCK_END && ptr< max)
		{
			*ptr=0x00;
			ptr++;
		}
		if (ptr < max)
		*ptr=0x00;
	}
}
	
