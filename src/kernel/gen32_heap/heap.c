#include "heap.h"
/*struct heap_table_entr
{
	size_t size;
	uint32_t* addr;	//points to address of next free block, 0x00 for last block
	bool free;
	uint32_t flags;
	};
*/	
extern void disk_coalescing();
void heap_init()
{
	//this is to set the first block 
	struct heap_table_entr* a=(struct heap_table_entr*)(HEAP_START);
	a->addr_n=0x00;
	a->free=true;
	a->size=(size_t)HEAP_SIZE-0x10; //size of block is total size minus size of header
	
	//this should set our first and only block at time of initialization
	}
	
void *default_case(struct heap_table_entr* cur,size_t size) //we expect cur to be pointing to a valid heap_table_entr and the last one
{
	//no blocks allocated so we allocated , we are first :0
		//this is the code for allocating some space at end
		if(cur->size+size+HEAP_HEADER > HEAP_SIZE)
			return NULL;
		struct heap_table_entr* next; // when we create a new block by splitting an existing one, we need to create a new one
		
		//modify current and update next
		cur->addr_n=(struct heap_table_entr*)((uintptr_t)cur+HEAP_HEADER+size);	//sets next 
		cur->addr_n=next;	//sets location for next block entry
		
		next->size=cur->size-(HEAP_HEADER+size); //sets size for next block as size of allocated space and space of next minus total free space
		cur->size=size;		//sets current size to requried size
		
		cur->free=false;	//sets free to false because block is allocated
		next->free=true;
		
		next->addr_n=0x00; //makes next the last block
		return (void*)((uintptr_t)cur+HEAP_HEADER);
 
	}
void *favourable_worst(struct heap_table_entr* cur,size_t size) //assume it is in between
{
	//code block for  most favourable sceanrio
	//it will also codefor worst because the logic of occuping the entire space 
	//between two block becomes favourable when the size of block is exactly the required size
	//and it becomes worst case when size is greater than required 
	//but the logic of consuming entire space between 2 blocks are same so this function 
	//can be used in both sceanrios, good or  bad depends on the scenario in which it is called
	
	cur->free=false;
	return (void*)((uintptr_t)cur+HEAP_HEADER);
	}
void *fair(struct heap_table_entr* cur,size_t size)
{
	//this is the faur case here we spilt the size between two exitsting blocks in hope that the 
	//newly created free block wil  be used later
	
	struct heap_table_entr* next=(heap_table_entr*)((uintptr_t)cur+size+HEAP_HEADER);//sets next
	
	//upadate addresses
	next->addr_n=cur->addr_n;
	cur->addr_n=next;
	
	//upadate size
	next->size=cur->size-(HEAP_HEADER+size);
	cur->size=size;
	
	//free 
	next->free=true;
	cur->free=false;
	//this function should have updated cur with the same size as required and free to false
	//created a newe block of size curr_size-12-size, and free to true
	return (void*)((uintptr_t)cur+HEAP_HEADER);
	}
void * malloc(size_t size)
{
	if(size ==0)
		return NULL;
	size= (size+0xf) & ~(0xf); //adds 0xf or 15,so if it is perfectly divisible by 16 last 4bis are 0 so 
	//it does not affect bit 5, but if it is even 1 greater than mulitple of 16,then bit 5 is incremented
	//then we and the lower 4bits with zero (~0xf), means all one except last four, so it becomes a muliple 
	//of 16 again, if it was a muliple of 16 then bit 5 is not incremented else it will be
	struct heap_table_entr* cur=(struct heap_table_entr*)(HEAP_START); 
	struct heap_table_entr* cand=NULL;
	//our malloc algo here ;)
	while(cur->addr_n!=0x00)
	{
		if(cur->free==true)
		{
			if (cur->size == size)	//priority one
				return favourable_worst(cur,size);
			else if (cur->size > size+HEAP_HEADER+HEAP_MIN_LIMIT)	//priority two
				{
					if(cand ==NULL)			//updates if cand is null
						cand=cur;
					else
						if(cand->size < cur->size)	//changes only to a lower vale of priority 2
						cand =cur;
				}
			else if (cur->size ==size+HEAP_HEADER+HEAP_MIN_LIMIT && cand==NULL) //only accepts to prevent from going to default
				cand=cur;
		}
		cur=cur->addr_n;
	}
	if (cand==NULL)
		return default_case(cur,size);
	else if(cand->size > size+HEAP_HEADER+HEAP_MIN_LIMIT)
		return fair(cand,size);
	else
		return favourable_worst(cand,size);
}

void free(void* ptr)
{
	struct heap_table_entr* pt1=(struct heap_table_entr*)((uintptr_t)ptr-HEAP_HEADER);
	pt1->free=true;
	void disk_coalescing();
	}

