#include "paging.h"
#include "../heap/heap.h"
dir_table_address create_32_dir_table(uint32_t flags)
{
	//this function creates a directory table by allocating space(4kb) for itself
	//then allocate 4kb upto 1024 times , so 4mb addtionally, and move the returned addresses
	//to the corresponding entries to page directory table,
	dir_table_address rtn_val= (dir_table_address)kzalloc(NO_DIR_ENTRIES*sizeof(dir_table_entries));
	dir_table_entries index_val=0x00;
	dir_table_address index=0x00;
	for(int i=0;i<NO_DIR_ENTRIES;i++)
	{//should perform pointer arithemetic, so add 4 instead of 1 every single time ot rtn_val
		index=rtn_val+i;
		index_val=(dir_table_entries)kzalloc(NO_PAGE_ENTRIES*sizeof(page_table_entries)); 
		//kzalloc desingned to return 4kb aligned addresses
		//as address returned is a mulitple of 4096
		//last 12 bits are aldready zero but quick asserting is cheap and helps in later debugging
		
		*index=((uint32_t)index_val & 0xfffff000)| (flags& 0xfff);;
		
		//code for setting flags
		
		}
	return rtn_val;
	
	}
