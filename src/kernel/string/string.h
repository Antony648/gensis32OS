#ifndef STRING_H
#define  STRING_H
#include <stddef.h>
#include <stdbool.h>
int strlen(const char* str);
char* strcpy(char* dest,const char*src);
char* strncpy(char* dest,const char*src,size_t n);
int strnlen(const char* str,int n);
int strncmp(const char* s1,const char* s2,size_t n);
int strcmp(const char*s1,const char*s2);

#endif
