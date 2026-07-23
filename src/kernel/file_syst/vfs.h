#ifndef  VFS_H
#define VFS_H
#include <stdint.h>
#include "../error.h"
#include "../disk/disk.h"
#include "partitions.h"
#include "../osconfig.h"
#include "stdbool.h"
#include <stddef.h>

#define file_read 0x1
#define file_write 0x2
#define file_exec 0x4
#define CTE_MOUNT_PNT   0x0001
#define CTE_FILE        0x0010
#define CTE_DIR         0x0100
#define CTE_ROOT        0X1000
struct mount_table_entry;
struct vfs_node;
struct cache_table_entry{

    uint64_t path_hash;
    int content_type;
    union c
    {
        struct mount_table_entry *mnt_tbl_entry_ptr;//for mount points
        struct vfs_node *vfs_node_ptr;  //for normal files
    }content;
    uint16_t refcount;
    uint16_t flags;
    
                        //to origin to get partition, bpb
    struct cache_table_entry* next;
    struct cache_table_entry* prev;

};  //size  64 bytes

struct file{
    struct vfs_node* vfs_node_ptr;
    uint8_t flags;      //read 0x1 write 0x2 exectute 0x4
    uint32_t offset;

};
struct mount_table_entry{
    struct partition* mnt_part;
    struct vfs_node* fs_root_node;
    struct cache_table_entry *ct_table_entry;
    void* fs_bpb;   //generally for bpb of any file_system
    struct mount_table_entry* next;
    struct mount_table_entry* prev;

};



struct vfs_node{
   
    uint32_t size;  //data stored as byte count there by making largest file size about 4GB 
    uint32_t node_id; //unique id

    uint16_t mode;
    uint16_t user_id;
    uint32_t dirty_bit; //i used 32bit value for alignment, dirty bit is used in situvation such as write
        //or delete where file attributes may have changed and it has to be updated in file entry in partition
        //we should later write a process that scans all vfs nodes and will write back file data to partition
        //where dirty bit is high and sets it low
    uint32_t access_time;
    uint32_t modified_time;
    union k
    {
        struct cache_table_entry* ct;    
        struct mount_table_entry* mte;
    }content;
    struct mount_table_entry *origin_mount_point; //for normal file pointing 
    uint32_t fs_specific;      //fat16 data section cluster number;
    
   

};

struct fops{
    uint32_t (*get_root_specific)(struct vfs_node* node, struct partition* part);
    int (*get_file_specific)(struct vfs_node*node, struct vfs_node* node1, char* name);
    int (*vfs_write)(struct file* file_ptr,char* buffer,uint32_t size);
    int (*vfs_read)(struct file* file_ptr,char* buffer,uint32_t size);
    //void* (*parse_partition_fill_bpb)(struct partition* part);
    int (*vfs_open)(char* path);
    int (*vfs_close)(char* path);
    int (*vfs_create_file)(char* path,uint8_t type);
    int (*vfs_delete_file)(char* path);
    int (*mount)(struct partition* part,char* path,struct vfs_node* parent);
    int (*umount)(char* path);
};
struct file_system{
    char* name;
    struct fops fopse;
};

int vfs_mount(struct partition* par,char* path,struct vfs_node* node);
struct file* open_file(char* path);
int close_file(struct file* file_ptr);
int read_file(struct file*,char* buffer,uint32_t size,uint32_t offset);
int write_file(struct file*,char* buffer,uint32_t size,uint32_t offset);
int generate_vfs_node_id();
struct cache_table_entry* it_all_exist_but_one(const char*);
size_t get_me_last_head(const char*);
uint32_t get_fs_specific_cache_table(struct cache_table_entry* ct);
#endif
