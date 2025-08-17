#include "essentials.h"

void* memset(void* s, int c, size_t n)
{
	uint8_t* s1=(uint8_t*)s;
	for(size_t i=0;i<n;i++)
		s1[i]=(uint8_t) c;
	return s;
}

void* memset_fast(void*	s,int c,size_t n) //writes 4bytes at a time so faster
{
	uint32_t* s1=(uint32_t*)s; //
	size_t n1=n/4;
	uint32_t val;
	uint32_t* ptr=& val;
	memset( ptr,c, 4);	//after this call val will contain 4bytes repeated values of that particular bit

	for(size_t i=0;i<n1;i++)
		s1[i]=val;
	ptr=(void*)(&s1[n1]);
	memset(ptr,c,n%4);
	return s;

}

void* gen_memset(void* s, int c, size_t n)
{
	if (((uintptr_t)s%4)==0)              //typecasts s to an unsigned integer pointer value type that can used to perform arithemetic operations enabling us to perfrom arithemetic operations
		memset_fast(s,c,n);
	else
		memset(s,c,n);
	return s;
}

int memcmp(void *s1,void*s2,size_t n)
{
	uint8_t* p1=(uint8_t*)s1;
	uint8_t * p2=(uint8_t*)s2;
	for(;n--;p1++,p2++)
		if(*p1 !=*p2)
			return (int)(*p1-*p2);
	return 0;
}
void* memcpy(void *dest, void* src,size_t n)
{
	uint8_t * p1=(uint8_t *)dest;
	uint8_t * p2=(uint8_t*)src;
	for(;n;p2++,p1++,n--)
		*p1=*p2;
	return dest;
}
