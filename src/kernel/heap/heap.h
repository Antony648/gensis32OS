#ifndef HEAP_H
#define HEAP_H
#include "../osconfig.h"
#include <stdint.h>
#include <stddef.h>

typedef unsigned char heap_block_entry;

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN	0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE		0x00
#define HEAP_BLOCK_HAS_NEXT				0x80
#define HEAP_BLOCK_IS_FREE				0x40
struct heap_table	//insted of an array ,a struct containing start address 
					// and number of elements, we can implement this by creating a pointer and 
					// and assigning it to a physical location in ram and then access as offset from there
{
	HEAP_BLOCK_ENTRY* entries;
	size_t total;
	};
	
struct heap
					//this is a struct that actually refers to a heap, 
					//it contains a heap table and start and end addresses
{
	struct heap_table* table;
	void* start_addr;
	void* end_addr;
	
	};
#endif
