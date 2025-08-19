#include "heap32.h"
#include "../error.h"
#include "../kernel.h"
void table_guard_set(void* page_addr)
{
	uint8_t* table=(uint8_t*)page_addr;
	table[124]=0xab;
	table[125]=0xab;
	table[126]=0xab;
	table[127]=0xab;
}
static int get_index(void* page_addr,uint32_t no_blocks)
{
	//search for continuous blocks of no_blocks
	//first fit allocation
	uint8_t* table_index=page_addr;
	int i=0;
	int rtn_val=-1;
	int target=no_blocks;
	for(;i<124;i++)  //only 124 available space 
	{
		 if(table_index[i]== 0x00)
		 {
			 rtn_val=i;
			 i++;target--;
			 for(;target && i<124;i++,target--)
			 {
				 if(table_index[i]!= 0x00)
					break;
			 }
			 if(!target)
				return rtn_val;
			 else
				{	//reset
					target=no_blocks;rtn_val=-1;
				}
		 }
	}
	return rtn_val;
	
}
static int set_table_val(void* page_addr,int index,uint32_t no_blocks)
{
	uint8_t* index_addr=(uint8_t*)page_addr+index;
	if(no_blocks ==1)
	{
		if(* index_addr !=0x00)
			return GEN32_INVARG;
		*index_addr = 0x41;
		return 0;
	}
	
	if(*index_addr != 0x00)
		return GEN32_INVARG;
	*index_addr =0xc1;
	//first block set
	int i=1;
	for(;i<no_blocks -1;i++)
	{
		//all middle blocks
		if(index_addr[i]!=0x00)
			return GEN32_INVARG;
		index_addr[i]=0x81;
	}
	//last one block
	if(index_addr[i] !=0x00)
		return GEN32_INVARG;
	index_addr[i]=0x01;
	//success
	return 0;
}
void* malloc_32(void* page_addr,size_t size)
{
	//first 128 bytes are table
	uint32_t no_blocks=(uint32_t)((size+31)>>5);
	
	int index=get_index(page_addr,no_blocks);
	if(index <0)
	{
		print("heap32:could not get a block of that size\n");
		return NULL;
	}
	//we have index at this point actual address
	if(set_table_val(page_addr,index,no_blocks)<0)
	{
		print("heap32.c:function failure");
		return NULL;
	}
	//calculate actual address and send to exit
	return (void*)((uint8_t*)page_addr+(DATA_OFFSET_BYTES+index* SIZE_OF_BLOCK_BYTE));
	
}
bool heap32_free(void* page_addr, void* block_addr)
{
	//uint32_t* addr=(uint32_t*)block_addr;
	//check
	if((uint8_t*)block_addr-(uint8_t*)page_addr<DATA_OFFSET_BYTES)
	{	print("heap32.c:heap32_free:invalid arg");
		return false;
	}
	
	int index= (uint8_t*)block_addr-(uint8_t*)page_addr-DATA_OFFSET_BYTES;
	uint8_t* index_addr=(uint8_t*)page_addr+(index/32);
	//for abnormal cases
	if(*index_addr == 0xab)
	{
		print("hit table end.\n");
		return false;
	}
	
	if(*index_addr ==0x00)
	{
		print("double free\n");
		return false;
	}
	if(*index_addr ==0x01 || *index_addr ==0x81)
	{
		print("incorrect\n");
		return false;
	}
	//for normal cases
	if(*index_addr ==0x41)
	{
		*index_addr=0x00; return true;
	}
	else
	{
		if(*index_addr!=0xc1)
		{
			print("corrupted head\n");
			return false;
		}
		*index_addr =0x00;
		index_addr++;
		//destroyed dragon's head
		for(;*index_addr!=0x01 && (index_addr-(uint8_t*)page_addr)<DATA_OFFSET_BYTES;index_addr++)
		{
			//loop for destroying dragon's body full of 0x81
			if(*index_addr!=0x81)
			{
				print("corrupted mid\n"); return false;
			}
			*index_addr=0x00;
			
		}
		//for destroying dragon's tail 0x01
		*index_addr=0x00;
	}
	return true;
}
