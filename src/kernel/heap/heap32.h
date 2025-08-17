#ifndef HEAP_32_H
#define HEAP_32_H
#include "kheap.h"
#include <stddef.h>
#include <stdint.h>
//this is strictly meant for allocating size for blocks
//between 1-4kb,less than a page, the whole idea is to break
//the existing 4kb chunk into 32byte blocks

#define DATA_OFFSET_BYTES  128
#define DATA_OFFSET_BLOCK  4
#define SIZE_OF_BLOCK_BYTE	32
#define BLOCK_START	0xc1
#define BLOCK_END	0x01
#define BLOCK_SINGLE 0x41
#define BLOCK_MID	0x81
#endif
