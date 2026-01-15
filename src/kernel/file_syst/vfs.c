#include "./vfs.h"
#include <stdint.h>
uint32_t VFS_INODE_ID_COUNT=0;
uint32_t CACHE_TABLE_COUNT=0;
struct mount_table_entry* general=0x00;
struct cache_table_entry CACHE_TABLE[MAX_OPEN_FILE_COUNT];
int vfs_mount(struct partition* par,char* path,struct vfs_node* node){

}

struct file* vfs_open(char* path){

}
int vfs_close(struct file* file_ptr){

}
int generate_vfs_node_id(){
	if(VFS_INODE_ID_COUNT<VFS_NODE_ID_MAX)
		VFS_INODE_ID_COUNT++;
	else
		return -VFS_NODE_DEPLETION_PANIC;
}
struct cache_table_entry* it_all_exist_but_one(const char* path){
	return (struct cache_table_entry*)0x00;
}
size_t get_me_last_head(const char* path){
	size_t rtn_val=0;
	for(int i=0;path[i];i++)
	{
		if(path[i]=='\\')
			rtn_val=i;
	}
}