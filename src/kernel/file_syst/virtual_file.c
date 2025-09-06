#include "./virtual_file.h"
#include "../string/string.h"
#include <stddef.h>
#include "../ctype/ctype.h"
#include "../heap/heap_cream.h"
#include <stdint.h>

uintptr_t karray_vfs[5]={0,0,0,0,0};
extern struct disk* motherlobe[5];

//implement core vfs 
struct mount_point_ent* mindflayer[MAX_MOUNT];

void os_native_main_mount()
{
	//we aim to set fat16 root (os native) as our main mount at /
	struct mount_point_ent* main=(struct mount_point_ent*)heap_cream_malloc(karray_vfs, sizeof(struct mount_point_ent));
	char target[2]="/";
	main->path=target;
	main->disk=motherlobe[0];
	//should write the functions for getting file desc of the root from fat 16 and assign it to main
	//for now 
	main->root=NULL;
}
void init_mindflayer()
{
	// initialses rest to NULL
	for(uint8_t i=0;i<MAX_MOUNT;i++)
		mindflayer[i]=NULL;
	
}



struct mount_point_ent* get_mount_point(const char* path)
{
	
	for(uint8_t i=0;i<MAX_MOUNT;i++)
	{
		if(mindflayer[i]==NULL)
			continue;
		if(!strncmp(mindflayer[i]->path, path,(size_t)strlen(path)))
		{
			return mindflayer[i];
		}
	}
	return NULL;	
}
struct file_desc* vfs_open(const char* cur_dir,const char* path)
{
	
	char* ppath;
	if(isdigit(path[0]))		//contains full path with disk id can work with, not general
		ppath=(char*)path;
	else
	{
		uint32_t len1=strlen(cur_dir);
		uint32_t len=strlen(path);

	}

}