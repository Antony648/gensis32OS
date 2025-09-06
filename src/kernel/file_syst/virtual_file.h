#ifndef VIRTUAL_FILE_H
#define VIRTUAL_FILE_H
#include "../disk/disk.h"
#include <stdint.h>
#include "../date_time/date_time.h"
#define NAME_MAX 225	
#define MAX_MOUNT 32
enum FILE_TYPE
{
	DATA_FILE,
	SUB_DIR,
	SYMLINK,
	HRDLINK,
	DEVICE_FILE,
	HIDDEN_FILE,
	SYSTEM_FILE,
};
enum PERMISSIONS
{
	READONLY=4,
	WRITEONLY=2,
	EXECONLY=1,
	READ_AND_WRITE=6,
	READ_AND_EXEC=5,
	WRITE_AND_EXEC=3,
	ALL=7,

};
struct file_desc; 	//forward decclaration


struct mount_point_ent
{
	struct file_desc* root;		//subdirectory that is going to be mounted
	const char* path;					//path in currently mounted directory where we mount the subdirectory
	struct disk* disk;		
};
struct file_operations
{
	int (*open)(struct file_desc* file);
	int (*close)(struct file_desc* file);
	int (*read)(struct file_desc* file,uint32_t offset,uint32_t len,char* buffer);
	int (*write)(struct file_desc* file,uint32_t offset,uint32_t len,char* buffer);

};
struct vfs_timestamp
{
	struct date vfs_date;
	struct time vfs_time;
};
struct file_desc
{
	enum FILE_TYPE type;
	char name[NAME_MAX];
	uint32_t file_id;
	uint32_t size;
	struct vfs_timestamp create;
	struct vfs_timestamp update;
	struct vfs_timestamp accessed;
	void* fs_hook;
	enum PERMISSIONS permissions;
	const struct file_operations* ops;
	struct mount_point_ent* mnt_pnt;	//could have used a uint8_t and used it for storing disk no
										//but i think it would be too much low level for a file for disk info 
										//i wanted to abstract it away.....
};
struct mount_point_ent* get_mount_point(const char* path);
#endif
