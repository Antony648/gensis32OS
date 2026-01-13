#include "vfs.h"
#include "partitions.h"
#include <stdint.h>
#include "../heap/heap_cream.h"
extern uint8_t G_ROOT_DRIVE;
extern uint8_t G_ROOT_PART;
extern struct disk* motherlobe[DISK_SUPPORT_MAX];
struct mount_table_entry mount_table[FS_MOUNTS_MAX];
struct cache_table_entry cache_table[MAX_OPEN_FILE_COUNT];
void mount_root(struct partition* part)
{
	struct disk* disk_ptr;
	for(uint8_t i=0;i<DISK_SUPPORT_MAX;i++)
	{
		disk_ptr=motherlobe[i];
		if(disk_ptr->ata_code== G_ROOT_DRIVE)
			break;
	}
	struct partition* part_ptr;
	
	part_ptr=disk_ptr->link_list;		//least possible value is 1 this operation is mandatory
	for(uint8_t i=1;i<G_ROOT_PART;i++)
	{
		if(part_ptr->next)
			part_ptr=part_ptr->next;
		else
			break;
	}

	cache_table[0].flags=CTE_MOUNT_PNT;
	cache_table[0].parent=NULL;
	struct vfs_node* node=heap_cream_malloc(sizeof(struct vfs_node));
	mount_table[0].mnt_part=part_ptr;
	mount_table[0].fs_root_node=node;
	node->fs_type=part_ptr->fs_type;
	node->fs_specific=0;
	cache_table[0].target=(void*)&mount_table[0];

}
struct file* vfs_open(char* path){

}
int vfs_close(struct file* file_ptr){
	return 0;
}
int mount(struct partition* part,char* path)
{

}
int umount(char* path)
{

}