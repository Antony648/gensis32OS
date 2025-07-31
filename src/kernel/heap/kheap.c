#include "kheap.h"
#include "heap.h"
#include "../kernel.h"
#include "../osconfig.h"
struct heap kernel_heap;
struct heap_table kernel_heap_table;
void kheap_init()
{
	kernel_heap_table.total=KHEAP_SIZE_BYTES/KHEAP_BLOCK_SIZE;
	kernel_heap_table.entries=(heap_block_entry*)KHEAP_TABLE;
	void* end=(void*)(KHEAP_START+KHEAP_SIZE_BYTES);
	if(heap_create(&kernel_heap,(void*)KHEAP_START,end,&kernel_heap_table))
		print("failed to create heap.\n");
	}
