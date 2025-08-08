#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
	//page directory entry is a 32 bit structure having
	//upper 20 bits of address to page table(12-31)
	//4 bits after that in AVL(8-11)
	//bit 7 for pagesize
	//bit 6 for remaining one bit of AVL
	//bit 5 for  Accessed bit
	//bit 4 for PCD page cache disable
	//bit 3 for PWT page write through
	//bit 2 for U/S user/supervisor
	//bit 1 for R/W read/write
	//bit 0 for present bit
	


	//page directory entry is a 32 bit structure having
	//upper 20 bits of address to page table(12-31)
	//3 bits after that in AVL(9-11)
	//bit 8 for GLOBAL
	//bit 7 for PAT page attribute
	//bit 6 for Dirty bit (to check if block data is modified)
	//bit 5 for  Accessed bit
	//bit 4 for PCD page cache disable
	//bit 3 for PWT page write through
	//bit 2 for U/S user/supervisor
	//bit 1 for R/W read/write
	//bit 0 for present bit
//lower 5 bits have the same purpose for bit ,so we can share bit maps
typedef uint32_t dir_table_entries;
//if things go right it should be a 32 bit value,
typedef  dir_table_entries* dir_table_address ;

typedef uint32_t 	page_table_entries;
typedef  page_table_entries* page_table_address;

dir_table_address create_32_dir_table(uint32_t flags);
void set_dir_table(dir_table_address addr);
void enable_paging();

#define PAGING_PRESENT		0b00000001
#define READ_AND_WRITE		0b00000010
#define ACCESS_ALL 			0b00000100
#define PAGE_WRITE_THROUGH	0b00001000
#define PAGE_CACHE_DISABLE	0b00010000

#define NO_DIR_ENTRIES		1024
#define NO_PAGE_ENTRIES		1024
#endif
