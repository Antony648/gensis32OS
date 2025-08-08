#include "paging.h"
#include "../kernel.h"
#include "../heap/heap.h"

extern load_dir_table(dir_table_address);
extern enable_paging_asm();
static dir_table_address  cur_dir_table=NULL;
dir_table_address create_32_dir_table(uint32_t flags)
{
	//this function creates a directory table by allocating space(4kb) for itself
	//then allocate 4kb upto 1024 times , so 4mb addtionally, and move the returned addresses
	//to the corresponding entries to page directory table,
	dir_table_address rtn_val= (dir_table_address)kzalloc(NO_DIR_ENTRIES*sizeof(dir_table_entries));
	dir_table_entries index_val=0x00;
	uint32_t* page_index=NULL;
	if(rtn_val ==NULL)
	{
		print("create_32_dir_table,paging.c:page dir table was itself given NULL");
		return NULL;
	}
	else if((((uintptr_t)rtn_val)& 0xfff)!=0)
	{
		 print("create_32_dir_table,paging.c:page dir table was itself given unaligned addr");
		 return NULL;
	}
	dir_table_address index=rtn_val;
	uint32_t offset=0x00;
	for(int i=0;i<NO_DIR_ENTRIES;i++)
	{
		index_val=(dir_table_entries)kzalloc(NO_PAGE_ENTRIES*sizeof(page_table_entries)); 
		//dir_table_entries is a type def to uint32_t
		//kzalloc desingned to return 4kb aligned addresses
		//as address returned is a mulitple of 4096
		//last 12 bits are aldready zero but quick asserting is cheap and helps in later debugging
		//index[i] will increment  (4*i)bytes to index
		if ((uint32_t*)index_val==NULL)
		{
			print("create32_dir_table,paging.c:kzalloc in dir table gave NULL\n");
			index[i]=0;
		}
		else if((index_val & 0xfff)!=0x00)
		{
			print("create32_dir_table,paging.c:kzalloc in dir table gave non aligned address\n");
			index[i]=0;
		}
		else
		{
			index[i]=(index_val & 0xfffff000)| (flags& 0xfff)| READ_AND_WRITE|PAGING_PRESENT;
			//sets the flags and read write for directory entry
			page_index=(uint32_t*)index_val;
			for(int j=0;j<NO_PAGE_ENTRIES;j++)
			{
				page_index[j]=(offset)|(flags& 0xfff)| READ_AND_WRITE|PAGING_PRESENT;
				offset+=0x1000;
			}
			
		}
		
		//code for setting flags
		
	}
	return rtn_val;
	
	}
void set_dir_table(dir_table_address addr)
{
	load_dir_table(addr);	//asm function that sets the cr3
	cur_dir_table=addr;
	
}

void enable_paging()
{
	//checks if any
	enable_paging_asm();
}
