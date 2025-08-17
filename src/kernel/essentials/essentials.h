#ifndef  ESSENTIALS_H
#define  ESSENTIALS_H

#include <stdint.h>
#include <stddef.h>
void* memset(void*s, int c, size_t n);		//exactly like memset from libc
void* memset_fast(void*	s,int c,size_t n); //writes 4bytes at a time so faster

void* gen_memset(void* s, int c, size_t n); //most preferred will pick form memset and memset_fast based on alginment
int memcmp(void *s1,void*s2,size_t n);
void* memcpy(void *dest, void* src,size_t n);
#endif
