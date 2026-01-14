#include "vfs.h"
#include "fat16.h"
#include "partitions.h"
#include <stdint.h>
#include "../heap/heap_cream.h"
extern uint8_t G_ROOT_DRIVE;
extern uint8_t G_ROOT_PART;
extern struct disk* motherlobe[DISK_SUPPORT_MAX];

struct mount_table_entry mount_table[FS_MOUNTS_MAX];
struct cache_table_entry cache_table[MAX_OPEN_FILE_COUNT];
uint8_t MOUNT_TABLE_COUNT=0;
uint8_t CACHE_TABLE_COUNT=0;
uint32_t VFS_NODE_ID_TRACK=0;
int generate_vfs_node_id()
{
	if(VFS_NODE_ID_TRACK<0xffffffff)
		return VFS_NODE_ID_TRACK++;
	else
	{
		//may this code never run, only runs if someone opens greater than 2^32-1 times, in a single session
		//tries to check for vfs_node id from 1367 onwards wanted to write a randno generator
		struct vfs_node* k;
		uint32_t sel;
		while(true)
		{
			sel=1367;int i;
			for(i=0;i<MAX_OPEN_FILE_COUNT;i++)
			{
				k=cache_table[i].target;
				if(!k)
					continue;
				else
				{
					if(k->node_id==sel)
					{
						sel++;
						break;
					}
				}
			}
			if(i==MAX_OPEN_FILE_COUNT)
				return  sel;
		}
	}
}
void destroy_cache_table_entry_single(struct cache_table_entry* ct)
{
	ct->flags=0;
	ct->parent=NULL;ct->target=NULL;
}
void init_cache_table()
{
	for(int i=0;i<MAX_OPEN_FILE_COUNT;i++)
		destroy_cache_table_entry_single(&cache_table[i]);
}

void destroy_mount_table_entry_single(struct mount_table_entry* mt)
{
	mt->fs_root_node=NULL;
	mt->mnt_part=NULL;
}
void init_mount_table()
{
	for(int i=0;i<FS_MOUNTS_MAX;i++)
		destroy_mount_table_entry_single(&mount_table[i]);
	
}
void mount_root(struct partition* part)
{
	CACHE_TABLE_COUNT++;MOUNT_TABLE_COUNT++;
	struct file_system* fs_ptr;
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

	cache_table[0].flags=CTE_ROOT;
	cache_table[0].parent=NULL;
	struct vfs_node* node=heap_cream_malloc(sizeof(struct vfs_node));

	cache_table[0].target=node;
	node->node_id=generate_vfs_node_id();
	mount_table[0].mnt_part=part_ptr;
	mount_table[0].fs_root_node=node;
	node->mte=&mount_table[0];
	//assumethat root fs is fat16 for now only fat16 support should write selection function later
	if(part_ptr->fs_type==FAT_16_LBA)
		fs_ptr=&fat16_fs;
	
	mount_table[0].fs_bpb=fs_ptr->fopse.parse_partition_fill_bpb(part_ptr);
	fs_ptr->fopse.get_root_specific(node,part_ptr);	//will populate the fs_speicific field of node based on part_ptr
	cache_table[0].target=(void*)&mount_table[0];

}
struct file* vfs_open(char* path){

}
int vfs_close(struct file* file_ptr){
	struct cache_table_entry* temp;

	struct vfs_node* node_temp=file_ptr->vfs_node_ptr;

	temp=node_temp->ct;
	temp->refcount--;
	heap_cream_free((void*)file_ptr);
	if(temp->refcount >0)
		return 0;
	if(temp->flags==CTE_MOUNT_PNT)
	{
		destroy_mount_table_entry_single((struct mount_table_entry*)temp->target);
		CACHE_TABLE_COUNT--;
	}
	heap_cream_free((void*)node_temp);
	destroy_cache_table_entry_single(temp);
	return 0;
}
int mount(struct partition* part,char* path)
{
	return 0;
}
int umount(char* path)
{
	//destroy fs bpb
	return 0;
}