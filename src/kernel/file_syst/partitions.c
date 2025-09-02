#include "./partitions.h"
#include "../string/string.h"
#include <stdbool.h>
uintptr_t karray[5]={0,0,0,0,0};
extern struct disk* motherlobe[5];

bool check_mbr(uint8_t* partition_table)
{
	if(!(partition_table[0]==0x80 || partition_table[0]==0x00))
		return false;
	switch((FILE_SYST_TYPE)partition_table[4])
	{
		
		case FAT_12:
		case FAT_16_L32:
		case FAT_16_G32:
		case FAT_16_LBA:
		case FAT_32_LBA:
		case LINUX_NATIVE:
			return true;
		default:
			return false;
	}
}
void fix_vbr(void* sect_0_buf,struct partition* head)
{
	
	
	const char* s1=(const char*)sect_0_buf+0x63;
	if(!strncmp(s1,"FAT12 ",6))
		head->fs_type=FAT_12;
	else if(!strncmp(s1,"FAT16 ",6))
		head->fs_type=FAT_16_LBA;
	else if(!strncmp(s1,"FAT32 ",6))
		head->fs_type=FAT_32_LBA;
	else
		head->fs_type=FS_UNKOWN;
}
struct partition* create_linked_list(struct disk disk1,void* sect_0_buf,uint8_t* partition_table)
{
	//analyse partition table generates a linked list and sends it 
	struct partition* head=NULL;
	if(!check_mbr(partition_table))
	{
		//code for vbr
		head=heap_cream_malloc(karray,sizeof(struct partition));
		fix_vbr(sect_0_buf,head);
		head->is_bootable=1;
		head->f_disk=disk1;
		head->next=NULL;
		return head;
	}
	//code for first entry
	struct partition* last=NULL;
	struct partition* cur=NULL;
	for(int i=0;i<4;i++)
	{
		partition_table=partition_table+(16*i);
		
		uint32_t *val=(uint32_t*)(partition_table+8);
		if(val[1]==0x00)	// 0 sector count means no entry 
			continue;
		
		cur=heap_cream_malloc(karray,sizeof(struct partition));
		if(partition_table[0]==0x80)
			cur->is_bootable=1;
		else
			cur->is_bootable=0;
		cur->fs_type=partition_table[4];
		
		cur->f_disk=disk1;
		cur->start_sect=val[0];
		cur->sect_num=val[1];
		cur->next=NULL;
		if(!head)
			head=cur;
		if(last==NULL)
			last=cur;
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
	void *sect_0_buf=heap_cream_malloc(karray,512);
	if(read_disk_block(disk_1,0, 1, sect_0_buf) < 0)
		goto exit_here;
	uint8_t* byte=(uint8_t*)sect_0_buf;
	if(byte[510]!=0x55 || byte[511]!=0xaa) 		//not an mbr or vbr
		goto exit_here;
	disk_1->link_list=create_linked_list(disk_1,sect_0_buf,byte+445);
exit_here:

	heap_cream_free(karray,sect_0_buf);
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
			single_disk_scan(motherlobe[i]);
	}
}
