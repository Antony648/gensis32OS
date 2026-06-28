#include "./vfs.h"
#include <stdint.h>
uint32_t VFS_INODE_ID_COUNT=-1;
uint32_t CACHE_TABLE_COUNT=0;
struct mount_table_entry* mount_table_start=0x0;
struct mount_table_entry* mount_table_last=0x00;
struct cache_table_entry* cache_table_start=0x00;
struct cache_table_entry* cache_table_last=0x00;
uint8_t cachet_empty_table[20];
int vfs_mount(struct partition* par,char* path,struct vfs_node* node)
{
	//analyse partition type and call accordingly
}

struct file* vfs_open(char* path)
{
	//check if already open , in cache table

	// break path using path parser, check if each subset is present from start,
	//if you reach a point of a file and if it is not present use specific of that 
	//file find the filesystem and use drivers to open that dir temporatrily
	//reapeat the process till you get final file

}

int vfs_close(struct file* file_ptr)
{
	//check for file in cache table, 
	//if found check for refcount, if zero
	//destroy file pointer,vfs node, cache table entry
}

int generate_vfs_node_id()
{
	if(VFS_INODE_ID_COUNT<VFS_NODE_ID_MAX)
	{
		VFS_INODE_ID_COUNT++;
		return VFS_INODE_ID_COUNT;
	}
	else
		return -VFS_NODE_DEPLETION_PANIC;
}

struct cache_table_entry* it_all_exist_but_one(const char* path)
{
	return (struct cache_table_entry*)0x00;
}

size_t get_me_last_head(const char* path)
{
	size_t rtn_val=0;
	for(int i=0;path[i];i++)
	{
		if(path[i]=='\\')
			rtn_val=i;
	}
}
uint32_t get_fs_specific_cache_table(struct cache_table_entry* ct)
{
	switch (ct->content_type) {
		case CTE_MOUNT_PNT:
			return ct->content.mnt_tbl_entry_ptr->fs_root_node->fs_specific;
		case CTE_FILE:
		case CTE_DIR:
		case CTE_ROOT:
			return ct->content.vfs_node_ptr->fs_specific;
		default:
			return 0;
	
	}
}