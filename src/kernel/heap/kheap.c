#include "kheap.h"
#include "heap.h"
#include "../kernel.h"
#include "../osconfig.h"
#include "../essentials/essentials.h"
struct heap kernel_heap;
struct heap_table kernel_heap_table;
void kheap_init()
{
	kernel_heap_table.total=KHEAP_SIZE_BYTES/KHEAP_BLOCK_SIZE;
	kernel_heap_table.entries=(heap_block_entry*)KHEAP_TABLE;
	void* end=(void*)(KHEAP_START+KHEAP_SIZE_BYTES);
	if(heap_create(&kernel_heap,(void*)KHEAP_START,end,&kernel_heap_table)<0)
		print("failed to create heap.\n");
}

void* kmalloc(size_t size)
{
		return heap_malloc(&kernel_heap,size);
}

void* kzalloc(size_t size)
{
	void* ptr=kmalloc(size);
	if (ptr!=NULL)
		memset(ptr,0x00,size);
	return ptr;
}

void kfree(void* ptr)
{
	return heap_free(&kernel_heap,ptr);
}
