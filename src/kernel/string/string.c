#include "./string.h"
int strlen(const char* str)
{
	uint32_t n=0;
	for(;*str;str++,n++);
	return n;
}	
char* strcpy(char* dest,const char*src)
{
	 char* rtn_val= dest;
	for(;*src;src++,dest++)
		*dest=*src;
	*dest='\0';
	return rtn_val;
}
char* strncpy(char* dest,const char*src,size_t n)
{
	 char* rtn_val= dest;
	for(;*src &&n--;src++,dest++)
		*dest=*src;
	while(n)
	{
		*dest='\0';dest++;n--;
	}
	return rtn_val;
}
int strnlen(const char* str,int n)
{
	uint32_t i=n;
	for(;i;i--,str++)
	{
		if(!(*str))
			return (int)(n-i);
	}
	return n;
}
int strncmp(const char* s1,const char* s2,size_t n)
{
	for(;n && (*s1 || *s2);n--,s1++,s2++)
	{
		if(*s1!=*s2)
			return (int)(*s1 -*s2);
	}
	return 0;
}
int strcmp(const char*s1,const char*s2)
{
	while(*s1 || *s2)
	{
		if(*s1 != *s2)
			return (int)(*s1 -*s2);
		s1++;s2++;
	}
	return 0;
}


