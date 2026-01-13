#include "./path_parser.h"
#include "./virtual_file.h"
#include "../string/string.h"
#include <stddef.h>
#include "../ctype/ctype.h"
#include "../heap/heap_cream.h"
#include "../disk/disk_stream.h"
#include "partitions.h"
#include "./fat16.h"
#include <stdint.h>


extern struct disk* motherlobe[5];
//extern struct file_operations* big_brother;
//implement core vfs 
struct mount_point_ent* mindflayer[MAX_MOUNT];

struct file_desc* find_in_oft(const char* path)
{
	
	//for now we implement looping later we will implement hash tables
	
	return NULL;
}


static int oemi_scan(struct partition* dexter)
{
	
	struct disk_stream* ds=init_disk_stream(dexter->f_disk);	//motherlobe is gaurnteed to always have os form which it is booted
 	disk_stream_seek( ds, dexter->start_sect*motherlobe[0]->sect_size +0x3);
 	char oemi[9]={0};
	get_bytes_from_disk(ds,8, (uint8_t*)&oemi);
	
 	free_disk_stream(ds);
 	return !strncmp(oemi, "GEN32 OS", 8);
}

struct partition* detect_native_part()
{
	//scans all partitions of motherlobe to find out which one is bootable
	struct partition* k=motherlobe[0]->link_list;
	struct partition* rtn_val=NULL;
	while(k)
	{
		enum FILE_SYST_TYPE k_fs=k->fs_type;
		if(k_fs==FAT_16_LBA || k_fs ==FAT_16_G32 || k_fs ==FAT_16_L32)
		{
			if(k->is_bootable)
			{
				if(!rtn_val)
					rtn_val=k;
				else      //two bootable fat16
				 if(oemi_scan(rtn_val))
				 	return rtn_val;
				 else
				 	rtn_val=k;

			}
		}
		
		k=k->next;
	}
	return  rtn_val;
}

struct mount_point_ent* os_native_main_mount()
{
	//we aim to set fat16 root (os native) as our main mount at /
	struct mount_point_ent* main=(struct mount_point_ent*)heap_cream_malloc(sizeof(struct mount_point_ent));
	strncpy(main->path, "/\0", 2);
	main->disk=motherlobe[0];
	//should write the functions for getting file desc of the root from fat 16 and assign it to main
	//for now 
	main->partition=detect_native_part();
	//we know native fs if fat16
	main->root=get_root_fat16(main->partition);
	return  main;

}

void init_mindflayer()
{
	// initialses rest to NULL
	for(uint8_t i=0;i<MAX_MOUNT;i++)
		mindflayer[i]=NULL;

	mindflayer[0]=os_native_main_mount();
	//this will give the first entry in the mount table,
	
}

void init_open_file_table()
{
	for(uint8_t i=0;i<MAX_FILE_COUNT;i++)	//MAX FILE COUNT is 128
		open_file_table[i]=NULL;
}



static struct mount_point_ent* single_scan(const char* block)
{
	//takes a slilced version or unsliced version and checks for mountpoint entry
	for(uint8_t i=0;i<MAX_MOUNT;i++)
	{
		if(mindflayer[i]==NULL)
			continue;
		if(strlen(mindflayer[i]->path)==strlen(block))
			if(!strncmp(mindflayer[i]->path, block,(size_t)strlen(block)))
			{
				return mindflayer[i];
			}
	}
	return NULL;
}
struct mount_point_ent* get_mount_point(const char* path)
{
	//tries if entire path is a mountpoint if not strip level by level till match found or main_mount(/)
	if(!strncmp(path, "/\0", 2))
		return mindflayer[0];
	struct mount_point_ent* rtn_val=NULL;

	if(path[0]!= '/')	//we expect absolute path, and all absolute paths have / at begining 
		return NULL; 

	char *use_path=(char*)heap_cream_malloc( PATH_MAX); //path max is 64 maximium size for a path in this os
	if(!use_path)
		return NULL;

	if(strlen(path)+1 > PATH_MAX)	//this makes it invalid
		goto exit;
	strncpy(use_path,path,strlen(path));
	use_path[strlen(path)]='\0';

	int i;
	while(use_path[0]!='\0')
	{
		rtn_val=single_scan(use_path);
		if(rtn_val)
			break;
		for(i=strlen(use_path)-1;use_path[i]!='/';i--); //finds the first occurance of / form back
		use_path[i]='\0';								//makes it the new end
	}
exit:
	heap_cream_free( use_path);
	return  rtn_val;
}

struct file_desc* vfs_open(const char* cur_dir,const char* path)
{
	const char* pass_val;
	struct file_desc* rtn_val;
	if(strncmp(cur_dir, path, strlen(cur_dir)))
	{
		//not full path
		char j[strlen(cur_dir)+ strlen(path)+1];	//create new array for full path
		strncpy(j, cur_dir, strlen(cur_dir));	
		strncpy(j+strlen(cur_dir),path,strlen(path));
		j[strlen(cur_dir)+ strlen(path)+1]='\0';
		//
		pass_val=j;

	}
	else
		pass_val=path;
	//pass_val at this point contians full path
	rtn_val=find_in_oft(path);
	//rtn_val->refcount++;
	if(rtn_val)
		return rtn_val;

	//if file not aldready open....
	struct path_head* snake=path_llist_gen(pass_val);
	//snake contains the mount point followed by each file to be opened
	big_brother=(struct file_operations*)snake->mnt_pnt->root->ops;	//the one in root is const struct file_operations*
	//context switch happens here...

}