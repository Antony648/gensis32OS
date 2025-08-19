#include "heap_cream.h"
#include "heap32.h"
#include "../kernel.h"
void heap_cream_init(uintptr_t* karray)
{
	for(int i=0;i<MAX_4KB_HEAP;i++)
		karray[i]=0;
}

void* heap_cream_malloc(uintptr_t* karray,size_t size)
{
	//uint32_t* karray=*karray_addr;
	if(size >TOTAL_BYTES_AVAIL)
	{
		print("use kzalloc for blocks greater than 3968\n");
		return NULL;
	}
	void* rtn_val=NULL;
	int i=0;
	for(;i<MAX_4KB_HEAP &&karray[i]!=0 ;i++)
	{
		rtn_val=malloc_32((void*)karray[i],size);	//tries to allocate pages in all tables
		if(rtn_val)
			return rtn_val;
	}
	//if control comes here we need to add a new block
	if(i==MAX_4KB_HEAP)
	{	
		print("reached max page limit");
		return NULL;		//we cannot do anything 
	}
	karray[i]=(uintptr_t)kzalloc(1);	//we allocate a page and give it to the last free address
	if(! karray[i])
	{
		print("kzalloc for new page failed");
		return NULL;
	}
	table_guard_set((void*)karray[i]);
	rtn_val=malloc_32((void*)karray[i],size);	//allocate space in new block
	return rtn_val;
}


static void heap_cream_kill(uintptr_t* karray,int index)
{
	kfree((void*)karray[index]);karray[index]=0;
}

/*static void heap_cream_check_kill(uintptr_t* karray)
{
	for(int i=0;i<MAX_4KB_HEAP;i++)
	{
		if(!karray[i])
		{
			uint8_t* table=karray[i];
			int j;
			for(j=0;j<124;j++)
			{
				if(table[j]!=0x00)
					break;
			}
			if(j==124)
				heap_cream_kill(karray,i);
		}
	}
}*/
static void heap_cream_single_chk_kill(uintptr_t *karray,int index)
{
	uint8_t* table=(uint8_t*)karray[index];
			int j;
			for(j=0;j<124;j++)
			{
				if(table[j]!=0x00)
					break;
			}
			if(j==124)
				heap_cream_kill(karray,index);
}

bool heap_cream_free(uintptr_t* karray,void* ptr)
{
		//we need to find which page block we have it
	if(ptr==NULL)
	{
		print("trying to free null.\n");
		return false;
	}
	int i=0;
	//uint32_t* karray=*karray_addr;
	uintptr_t base; uintptr_t p;
	for(;i<MAX_4KB_HEAP && karray[i]!=0  ;i++)
	{
		base=(uintptr_t)karray[i];p=(uintptr_t)ptr;
		if((p>=base) && (p-base <4096))	//pointer is inside that block
		{
			
			if(heap32_free((void*)karray[i],ptr))
			{
				//if we successfully freed the memory, we can now possibly free the page too...
				heap_cream_single_chk_kill(karray,i); 
				
				return true;
			}
		}
	}
	//print("heap_cream.c:heap_cream_free:invalid arg\n");
	return false;
}
void heap_cream_destroy(uintptr_t * karray)
{
	for(int i=0;i<MAX_4KB_HEAP &&karray[i] !=0; i++)
		{kfree((void*)karray[i]); karray[i]=0;}
}

