#ifndef	HEAP_CREAM_H
#define HEAP_CREAM_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define MAX_4KB_HEAP  128
void help_me_mary_mother_of_god();
//void heap_cream_init(uintptr_t* karray);
void* heap_cream_malloc(size_t size);
bool heap_cream_free(void* ptr);
//void heap_cream_destroy(uintptr_t * karray);

#endif
