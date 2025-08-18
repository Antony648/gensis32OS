#ifndef	HEAP_CREAM_H
#define HEAP_CREAM_H
#include <stdint.h>
#include <stddef.h>
#define MAX_4KB_HEAP  5

void* heap_cream_malloc(uint32_t**karray,size_t size);
void heap_cream_free(uint32_t** karray,void* ptr);

#endif
