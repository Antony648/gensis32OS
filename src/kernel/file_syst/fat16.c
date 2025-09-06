#include "./fat16.h"
#include "../disk/disk.h"
#include "../disk/disk_stream.h"
#include "../string/string.h"
#include <stdint.h>
#include "../heap/heap_cream.h"
#include "virtual_file.h"


uintptr_t karray_fat_16[5]={0,0,0,0,0};
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
	decode_datetime(&fd->update, dr->last_modified_date,dr->last_modified_date);
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
struct file_desc* get_root_fat16(uint32_t disk_id)
{
	//gets the file descriptor of a root partition using direct access
	struct disk_stream* ds=NULL;
	ds=init_disk_stream(disk_id);
	uint16_t res_sect,sect_per_fat,bytes_per_sect;
	uint8_t fat_copies;

 	disk_stream_seek(ds, 0x20b);
	get_bytes_from_disk( ds, 2, (uint8_t*)&bytes_per_sect);

	disk_stream_seek(ds, 0x20e);
	get_bytes_from_disk(ds, 2, (uint8_t*)&res_sect);

	disk_stream_seek(ds, 0x210);
	get_bytes_from_disk(ds, 1, (uint8_t*)&fat_copies);

	disk_stream_seek(ds, 0x216);
	get_bytes_from_disk(ds, 2, 	(uint8_t*)&sect_per_fat);

	//calculate the offset in disk where we can get data
	uint32_t root_dir_sect= res_sect +(fat_copies*sect_per_fat);
	root_dir_sect*= sect_per_fat;	

	//start of root dir sector in bytes or in other words offset in disk 
	//that has first  file's file_descriptor
	struct fat_16_root_dir_ent* first_fat16_ent=heap_cream_malloc(karray_fat_16,sizeof(struct fat_16_root_dir_ent));
	disk_stream_seek(ds, root_dir_sect);
	get_bytes_from_disk(ds, 32,(uint8_t*)first_fat16_ent);

	free_disk_stream(ds);
	struct file_desc* fat16_root=heap_cream_malloc(karray_fat_16, sizeof(struct file_desc));

	//we have file desc struct and root dir ent for first file

	fat16tovfs(fat16_root, first_fat16_ent);
	heap_cream_free(karray_fat_16 ,(void*)first_fat16_ent);

	return  fat16_root;
}
