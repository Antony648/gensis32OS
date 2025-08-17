#include "./ctype.h"
int isalpha(char c)
{
	if(( 65 <= c && c<= 90) || (97<= c && c<=122))
		return 1;
	else 
		return 0;
}
int isdigit(char c)
{
	if(48 <=c && c<=57)
		return 1;
	else 
		return 0;
}
int isalnum(char c)
{
	if(isalpha(c) || isdigit(c))
		return 1;
	else 
		return 0;
}
int isspace(char c)
{
	if(c==32 || (9<=c && c<=13))
		return 1;
	else 
		return 0;
}

char toupper(char c)
{
	return (97<=c && c<=122)?(char)(c & ~(0x20)):c;
}
char tolower(char c)
{
	return (65<=c && c<=90)?(char)(c | 0x20):c;
}
int isupper(char c)
{
	return (65<=c && c<=90)?1:0;
}
int islower(char c)
{
	return (97<=c && c<=122)?1:0;
}
int char_to_int(char c)
{
	return (int)(c-48);
}

