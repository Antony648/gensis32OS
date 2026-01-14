#ifndef  VFS_H
#define VFS_H
#include <stdint.h>

#include "../disk/disk.h"
#include "partitions.h"
#include "../osconfig.h"

#define file_read 0x1
#define file_write 0x2
#define file_exec 0x4
#define CTE_MOUNT_PNT   0x0111
#define CTE_FILE        0x0101
#define CTE_DIR         0x0010
#define CTE_ROOT        0X1000
struct cache_table_entry{

    struct cache_table_entry* parent;
    char name[FILE_NAME_LEN_MAX];
    void* target;   //it generally points to a vfs_node struct 
    //but can also point to a mount_table_entry
    uint16_t refcount;
    uint16_t flags;


};  //size  32 bytes

struct file{
    struct vfs_node* vfs_node_ptr;
    uint8_t flags;      //read 0x1 write 0x2 exectute 0x4
    uint32_t offset;

};
struct mount_table_entry{
    struct partition* mnt_part;
    struct vfs_node* fs_root_node;
    void* fs_bpb;   //generally for bpb of any file_system
    

};

#define BYTE    b'1'
#define KILO_B  b'10'
#define MEGA_B  b'100'
#define GIGA_B  b'1000'

struct vfs_node{
   
    uint32_t size;  //last 4 bits represent byte,kilobyte,megabyte,gigabyte
    uint32_t node_id; //unique id

    uint16_t mode;
    uint16_t user_id;

    uint32_t access_time;
    uint32_t modified_time;

    struct cache_table_entry* ct;
    void* fs_specific;      //generally an address or byte offset to a root sect dir ent in fat16
    struct mount_table_entry* mte;
   

};

struct fops{
    void (*get_root_specific)(struct vfs_node* node, struct partition* part);
    int (*get_file_specific)(struct vfs_node*node, struct vfs_node* node1, char* name);
    int (*vfs_write)(struct file* file_ptr,char* buffer,uint32_t size);
    int (*vfs_read)(struct file* file_ptr,char* buffer,uint32_t size);
    void* (*parse_partition_fill_bpb)(struct partition* part);
};
struct file_system{
    char* name;
    struct fops fopse;
};


struct file* vfs_open(char* path);
int vfs_close(struct file* file_ptr);
int mount(struct partition* part,char* path);
int umount(char* path);
#endif
