#include "./partitions.h"
#include "../string/string.h"
#include <stdbool.h>
#include "../kernel.h"
#include "../heap/heap_cream.h"
uintptr_t karray2[5]={0,0,0,0,0};
extern struct disk* motherlobe[5];
static uint32_t get32(const uint8_t* ptr)
{
	uint32_t rtn_val=0x00;
	rtn_val= (uint32_t)ptr[0]|((uint32_t)ptr[1]<<8)| ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3]<<24);
	return rtn_val;
}
static bool check_mbr(uint8_t* partition_table)
{
	
	//uint32_t sectors;
	uint8_t* loop_var=partition_table;
	for(int i=0;i<4;i++)
	{
		loop_var=partition_table+(i*16);
		//sectors=get32((const uint8_t*)(loop_var+12));
		if( loop_var[4]!=0x00 ) 
			return true;
	}
	return false;
}
static void fix_vbr(void* sect_0_buf,struct partition* head)
{
	
	
	const char* s1=(const char*)(sect_0_buf+0x36);
	if(!strncmp(s1,"FAT12 ",6))
		head->fs_type=FAT_12;
	else if(!strncmp(s1,"FAT16 ",6))
		head->fs_type=FAT_16_LBA;
	
	else if(!strncmp((const char*)sect_0_buf+0x52,"FAT32 ",6))
		head->fs_type=FAT_32_LBA;
	else if(!strncmp((const char*)sect_0_buf+0x47,"FAT32 ",6))
		head->fs_type=FAT_32_LBA;
	else
		head->fs_type=FS_UNKNOWN;
	head->start_sect=0;

}
static struct partition* create_linked_list(struct disk* disk1,void* sect_0_buf,uint8_t* partition_table)
{
	//analyse partition table generates a linked list and sends it 
	struct partition* head=NULL;
	if(!check_mbr(partition_table))
	{
		//code for vbr
		print("we have vbr\n");
		head=heap_cream_malloc(karray2,sizeof(struct partition));
		fix_vbr(sect_0_buf,head);
		head->is_bootable=0;
		head->f_disk=disk1;
		head->sect_num=disk1->sect_count;
		head->next=NULL;
		return head;
	}
	//code for first entry
	
	struct partition* last=NULL;
	struct partition* cur=NULL;
	uint32_t s_c_local;
	uint32_t s_s_local;
	uint8_t* loop_var=partition_table;
	for(int i=0;i<4;i++)
	{
		loop_var=partition_table+(16*i);
		
		s_s_local=get32((const uint8_t*)(loop_var+8));
		s_c_local=get32((const uint8_t*)(loop_var+12));
		if(s_c_local==0x00)	// 0 sector count means no entry 
			continue;
		
		cur=heap_cream_malloc(karray2,sizeof(struct partition));
		if(loop_var[0]==0x80)
			cur->is_bootable=1;
		else
			cur->is_bootable=0;
		//cur->fs_type=partition_table[4];
		switch(loop_var[4])
		{
			case 0x01:
				cur->fs_type=FAT_12;
				break;
			case 0x04:
				cur->fs_type=FAT_16_L32;
				break;
			case 0x06:
				cur->fs_type=FAT_16_G32;
				break;
			case 0x0e:
				cur->fs_type=FAT_16_LBA;
				break;
			case 0x0c:
				cur->fs_type=FAT_32_LBA;
				break;
			case 0x0b:
				cur->fs_type=FAT_32_CHS;
				break;
			case 0x83:
				cur->fs_type=LINUX_NATIVE;	
				break;
			default:
				cur->fs_type=FS_UNKNOWN;
				break;
		}
		cur->f_disk=disk1;
		cur->start_sect=s_s_local;
		cur->sect_num=s_c_local;
		cur->next=NULL;
		if(cur->start_sect + cur->sect_num > cur->f_disk->sect_count)
		{
			print("corrupted\n");
			heap_cream_free(karray2,cur);continue; 
		}
		
		if(!head)
		{
			head=cur;
			last=cur;
		}
		else
		{
			last->next=cur;
			last=cur;
		}
	}	
	return head;
}
void single_disk_scan(struct disk* disk_1)
{
	//assingns the linked list structure with a linked list of partition structures
	if(disk_1==NULL)
		return;
	disk_1->link_list=NULL;
	void *sect_0_buf=heap_cream_malloc(karray2,512);
	if(read_disk_block(disk_1,0, 1, sect_0_buf) < 0)
		goto exit_here;
	uint8_t* byte=(uint8_t*)sect_0_buf;
	if(byte[510]!=0x55 || byte[511]!=0xaa) 		//not an mbr or vbr
		goto exit_here;
	print("create a linked list\n");
	disk_1->link_list=create_linked_list(disk_1,sect_0_buf,byte+446);
	
exit_here:

	heap_cream_free(karray2,sect_0_buf);
	return;
}
void scan_part_all_disks()
{
	//scans motherlobe and fills the link_list field of the disk
	for(int i=0;i<5;i++)
	{
		if(!motherlobe[i])
			continue;
		else
		{
			single_disk_scan(motherlobe[i]);
		}
	}
}
static void print_link_list(struct disk* disk1)
{
	if(disk1->link_list==NULL)
	{
		print("no linked list");
		return;	
	}
	struct partition* cur=disk1->link_list;
	while(cur)
	{
		switch(cur->fs_type)
		{
			case FAT_12:
				print("fat12\n");
				break;
			case FAT_16_L32:
			case FAT_16_G32:
			case FAT_16_LBA:
				print("fat16\n");
				break;
			case FAT_32_LBA:
			case FAT_32_CHS:
				print("fat32\n");
				break;
			case LINUX_NATIVE:
				print("linux native\n");
				break;
				
			default:
				print("file system unknown\n");
				break;
		}
		cur=cur->next;
	}
}
void partition_debug()
{
	for(int i=0;i<5;i++)
	{
		if(!motherlobe[i])
			continue;
		else
			print_link_list(motherlobe[i]);
	}
}
