#include "./fat16.h"
#include "../disk/disk.h"
#include "../disk/disk_stream.h"
#include "../string/string.h"
#include <stdint.h>
#include "../ctype/ctype.h"
#include "../heap/heap_cream.h"
#include "virtual_file.h"




static void decode_datetime(struct vfs_timestamp* vfs, uint16_t date,uint16_t time)
{
	vfs->vfs_date.day=date & 0x1f; //lower 5 bits
	vfs->vfs_date.month=(date>>5) & 0x0f; //6-9 bits
	vfs->vfs_date.year=((date>>9)& 0x7f) + 1980;

	if(!time)
		return;
	vfs->vfs_time.second=(time & 0x1f) * 2; //fat stores in 2 seconds
	vfs->vfs_time.minute=(time >>5)& 0x3f; //7bits
	vfs->vfs_time.hour=(time >>11)& 0x1f;
}
static void fat16tovfs(struct file_desc* fd,struct fat_16_root_dir_ent* dr)
{
	strncpy(fd->name,dr->name,8);
	strncpy(fd->name +8, dr->extension,3);

	fd->permissions= READ_AND_WRITE;

	fd->file_id=dr->first_cluster;
	fd->size=dr->file_size;
	//set date time
	decode_datetime(&fd->create, dr->create_date,dr->create_time);
	decode_datetime(&fd->update, dr->last_modified_date,dr->last_modified_time);
	decode_datetime(&fd->accessed, dr->last_access_date, 0);
	//filetype setting
	if(dr->attr & 0x10)
		fd->type=SUB_DIR;
	else
		fd->type=DATA_FILE;
	fd->ops=(const struct file_operations*)&fat16_fops;

}

int fat16_open(struct file_desc* file)
{
	return 0;
}
int fat16_close(struct file_desc* file)
{
	return 0;
}
int fat16_read(struct file_desc* file,uint32_t offset,uint32_t len,char* buffer)
{
	return 0;
}
int fat16_write(struct file_desc* file,uint32_t offset,uint32_t len,char* buffer)
{
	return 0;
}

struct file_desc* get_root_fat16(struct partition *partition)
{
	//the job is to create a fake file descriptor for root
	
	struct file_desc* rtn_val=heap_cream_malloc( sizeof(struct file_desc));
	rtn_val->type=SUB_DIR;
	rtn_val->fs_hook=NULL;
	rtn_val->file_id=0;
	strncpy(rtn_val->name, "/\0",2);
	rtn_val->parent=NULL;
	rtn_val->sibling=NULL;
	rtn_val->mnt_pnt=NULL; 
	rtn_val->ops=(const struct file_operations*)&fat16_fops;
	//the ways of reconginsing if a file descriptor is root is id=0 and if it is a mount point is mnt_pnt=NULL
	return rtn_val;

}
static int fat16_file_name_cmp(char* file_name,char* test_name,size_t len)
{
	

	for(int i=0;i<len;i++)
	{
		char k=(i<strlen(test_name))?toupper(test_name[i]):' ';
		if(toupper(file_name[i])!=k)
			return 0;
	}
	return 1;
}
struct file_desc* get_root_child(struct partition* partition,char* name,char* extension)
{
	//gets the file descriptor of an immediate child of a root,

	struct disk_stream* ds=init_disk_stream(partition->f_disk);
	uint16_t res_sect,sect_per_fat,bytes_per_sect,rootentcount;
	uint8_t fat_copies;

	uint32_t offset=partition->start_sect* partition->f_disk->sect_size;
 	disk_stream_seek(ds, offset +0x0b);
	get_bytes_from_disk( ds, 2, (uint8_t*)&bytes_per_sect);

	disk_stream_seek(ds, offset+ 0x0e);
	get_bytes_from_disk(ds, 2, (uint8_t*)&res_sect);

	disk_stream_seek(ds, offset + 0x10);
	get_bytes_from_disk(ds, 1, (uint8_t*)&fat_copies);

	disk_stream_seek(ds, offset + 0x011);
	get_bytes_from_disk(ds, 2, (uint8_t*)&rootentcount);

	disk_stream_seek(ds,  offset + 0x16);
	get_bytes_from_disk(ds, 2, 	(uint8_t*)&sect_per_fat);

	//calculate the offset in disk where we can get data

	uint32_t root_dir_sect= res_sect +(fat_copies*sect_per_fat);
	root_dir_sect*= bytes_per_sect;	

	//start of root dir sector in bytes or in other words offset in disk 
	//that has first  file's file_descriptor

	struct fat_16_root_dir_ent* first_fat16_ent=heap_cream_malloc(sizeof(struct fat_16_root_dir_ent));
	if(!first_fat16_ent)
		goto exit;
	struct file_desc* fat16_root_child=NULL;
	//scan all root entries to find the child

	uint16_t i=0;
	for(;i<rootentcount;i++)
	{
		disk_stream_seek(ds, root_dir_sect+(i*32));
		get_bytes_from_disk(ds, 32,(uint8_t*)first_fat16_ent);

		if(first_fat16_ent->name[0]== 0x00 || first_fat16_ent->name[0]== 0xe5)
			continue;
		if(fat16_file_name_cmp( first_fat16_ent->name,name,8) && fat16_file_name_cmp(extension, first_fat16_ent->extension,3))
			{
				fat16_root_child=heap_cream_malloc( sizeof(struct file_desc));
				if(!fat16_root_child)
					goto exit2;
				fat16tovfs(fat16_root_child, first_fat16_ent);
				break;
			}

	}
exit2:
heap_cream_free((void*)first_fat16_ent);
exit:
	free_disk_stream(ds);

	

	//we have file desc struct and root dir ent for first file

	
	

	return  fat16_root_child;
}
