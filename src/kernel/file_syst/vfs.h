#ifndef  VFS_H
#define VFS_H
#include <stdint.h>
#define FILE_NAME_LEN_MAX 32
#include "../disk/disk.h"
#include "partitions.h"
#define file_read 0x1
#define file_write 0x2
#define file_exec 0x4
enum MOUNT_POINT:uint32_t
{
    MOUNT_POINT_FALSE,
    MOUNT_POINT_TRUE
};
struct cache_table_entry{

    struct cache_table_entry* parent;
    char name[FILE_NAME_LEN_MAX];
    struct vfs_node* target;
    uint32_t refcount=0;

    enum MOUNT_POINT is_mount_point=MOUNT_POINT_FALSE;
    struct mount_table_entry* mnt_point=NULL;

};  //size  52 bytes
struct fops{};
struct file{
    struct vfs_node* vfs_node_ptr;
    uint8_t flags;      //read 0x1 write 0x2 exectute 0x4
    //uint32_t
};
struct mount_table_entry{
    struct block_device* blk_dev;
    struct vfs_node* fs_root_node;


};
#define BYTE    b'1'
#define KILO_B  b'10'
#define MEGA_B  b'100'
#define GIGA_B  b'1000'
struct vfs_node{
    enum FILE_SYST_TYPE fs_type;
    uint16_t size;  //last 4 bits represent byte,kilobyte,megabyte,gigabyte

    uint32_t fs_specific;

};
struct file* vfs_open(char* path);
int vfs_write(struct file* file_ptr,char* buffer,size_t size);
int vfs_read(struct file* file_ptr,char* buffer,size_t size);
int vfs_close(struct file* file_ptr);
int mount(struct block_device* bd,char* path);
int umount(char* path);
#endif
