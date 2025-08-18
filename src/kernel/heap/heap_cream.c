#include "heap_cream.h"
#include "heap_32.h"
int 
void* heap_cream_malloc(uint32_t**karray_addr,size_t size)
{
	uint32_t* karray=*karray_addr;
	void* rtn_val=NULL;
	int i=0;
	for(;karray[i]!=0 && i<MAX_4KB_HEAP;i++)
	{
		rtn_val=malloc_32((void*)karray[i],size);	//tries to allocate pages in all tables
		if(rtn_val)
			return rtn_val;
	}
	//if control comes here we need to add a new block
	if(i==MAX_4KB_HEAP)
		return NULL;		//we cannot do anything 
	karray[i]=(uint32_t)kzalloc(1);	//we allocate a page and give it to the last free address
	rtn_val=malloc_32((void*)karray[i],size);	//allocate space in new block
	return rtn_val;
}
void heap_cream_free(uint32_t** karray_addr,void* ptr)
{
		//we need to find which page block we have it
	int i=0;
	uint32_t* karray=*karray_addr;
	for(;karray[i]!=0 && i<MAX_4KB_HEAP;i++)
	{
		if(((uint8_t*)ptr -(uint8_t*)karray[i])<4096)	//pointer is inside that block
		{
			heap32_free((void*)karray_addr[i],ptr);
			return;
		}
	}
	print("heap_cream.c:heap_cream_free:invalid arg\n")
}
