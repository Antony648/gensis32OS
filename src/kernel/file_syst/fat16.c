#include "fat16.h"
#include "../disk/disk.h"
#include "../heap/heap_cream.h"
#include "vfs.h"
#include <stdint.h>
#include "../string/string.h"
#include "../error.h"
#include "hash.h"
#include "../essentials/essentials.h"
#include "../kernel.h"
extern struct mount_table_entry* mount_table_last;
extern struct mount_table_entry* mount_table_start;
extern uint32_t VFS_INODE_ID_COUNT;
extern uint32_t CACHE_TABLE_COUNT;
extern struct cache_table_entry* cache_table_start;
extern struct cache_table_entry* cache_table_last;
static inline  uint16_t get_2_bytes(uint8_t* ptr)
{
	//we require this because of endianess
	return (uint16_t)((ptr[1]<<8)|ptr[0]);
}
static inline uint32_t get_4_byte(uint8_t* ptr)
{
	return  (uint32_t)((ptr[2]<<24)|(ptr[1]<<16)|(ptr[0])<<8|ptr[0]);
}

struct fat16_bpb* parse_partition_fill_bpb_fat16(struct partition* part )
{
	//
	uint8_t* sect_0_buf=heap_cream_malloc(512);
	if(read_disk_block(part->f_disk, part->start_sect, 1, sect_0_buf)<0)
	{
		heap_cream_free(sect_0_buf); return NULL;
	}
	struct fat16_bpb* bpb_ptr=(struct fat16_bpb*)heap_cream_malloc(sizeof(struct fat16_bpb));
	bpb_ptr->bytes_per_sect=get_2_bytes(&sect_0_buf[0x0b]);
	if(bpb_ptr->bytes_per_sect != 512 &&
		bpb_ptr->bytes_per_sect!=1024 &&
		bpb_ptr->bytes_per_sect!=2048 &&
		bpb_ptr->bytes_per_sect !=4096)
		goto  fail;

	bpb_ptr->sect_per_clust=sect_0_buf[0x0d];
	if(bpb_ptr->sect_per_clust ==0)
		goto fail;

	bpb_ptr->reserved_sectors=get_2_bytes(&sect_0_buf[0xe]);
	bpb_ptr->fat_count=sect_0_buf[0x10];

	if(bpb_ptr->fat_count==0)
		goto fail;

	bpb_ptr->root_entry_count=get_2_bytes(&sect_0_buf[0x11]);
	bpb_ptr->sectors_per_fat=get_2_bytes(&sect_0_buf[0x16]);
	if(bpb_ptr->sectors_per_fat ==0)
		goto fail;

	bpb_ptr->fat_lba=part->start_sect+ bpb_ptr->reserved_sectors;
	bpb_ptr->root_lba=bpb_ptr->fat_lba+(bpb_ptr->fat_count* bpb_ptr->sectors_per_fat);
	uint32_t root_dir_count;

	root_dir_count=((bpb_ptr->root_entry_count * 32)+(bpb_ptr->bytes_per_sect-1))/bpb_ptr->bytes_per_sect;
	//(A+B-1)/B; A/B in rounded div format

	bpb_ptr->data_lba=bpb_ptr->root_lba+root_dir_count;
	heap_cream_free(sect_0_buf);
	return bpb_ptr;
fail:
	heap_cream_free(sect_0_buf);
	if(bpb_ptr) heap_cream_free(bpb_ptr);
	return NULL;
}

int mount_fat16(struct partition* part,char* path,struct vfs_node*node)
{

	if(CACHE_TABLE_COUNT>=MAX_OPEN_FILE_COUNT)
	{
		print("mount table full\n");
		return -MOUNT_TABLE_FULL;
	}
	uint64_t hash_value=generate_hash(path);
	struct cache_table_entry* i;
	for(i=cache_table_start;i;i=i->next)
	{
		if(i->path_hash==hash_value)
		{
			print("trying to mount on existing path\n");
			goto fail;
		}
	}
	struct mount_table_entry* mnt_tbl_e=NULL;
	mnt_tbl_e=heap_cream_malloc(sizeof(struct mount_table_entry));
	if(!mnt_tbl_e)
		goto fail;

	if(!mount_table_start)
	{
		mount_table_start=mnt_tbl_e;
		mount_table_last=mount_table_start;
		mnt_tbl_e->prev=NULL;
	}
	else
	{
		mount_table_last->next=mnt_tbl_e;
		mnt_tbl_e->prev=mount_table_last;
		mount_table_last=mnt_tbl_e;
	}
	mnt_tbl_e->next=NULL;

	struct fat16_bpb* bpb=NULL;
	bpb=parse_partition_fill_bpb_fat16(part);
	if(!bpb)
		goto fail;

	//root does not exist in fat16 one is supposed to fabricate it
	struct vfs_node* r_node=NULL;
	r_node=(struct vfs_node*)heap_cream_malloc(sizeof(struct vfs_node));
	if(!r_node)
		goto fail;

	mnt_tbl_e->fs_root_node=r_node;
	mnt_tbl_e->mnt_part=part;
	mnt_tbl_e->fs_bpb=(void*)0x0;

	//node
	r_node->node_id=generate_vfs_node_id();
	r_node->content.mte=mnt_tbl_e;
	uint32_t root_dir_count=bpb->data_lba-bpb->root_lba;
	r_node->size=bpb->bytes_per_sect* root_dir_count;
	heap_cream_free(bpb);
	//ct
	i=heap_cream_malloc(sizeof(struct cache_table_entry));
	i->refcount=1;
	i->content_type=CTE_MOUNT_PNT;
	i->content.mnt_tbl_entry_ptr=mnt_tbl_e;	//cachetableentry points to  mounttableentry which has vfsnodeptr
	i->path_hash=hash_value;
	CACHE_TABLE_COUNT++;
	if(!cache_table_start)
	{
		cache_table_start=i;
		cache_table_last=i;
		i->prev=NULL;
	}
	else
	{
		cache_table_last->next=i;
		i->prev=cache_table_last;
		cache_table_last=i;
	}
	i->next=NULL;

	return  0;
fail:
	if(mnt_tbl_e) heap_cream_free(mnt_tbl_e);
	if(bpb) heap_cream_free(bpb);
	if(r_node) heap_cream_free(r_node);
	return  -MOUNT_FAILURE;
}

int umount_fat16(char* path)
{
	uint64_t hash_value=generate_hash(path);
	struct cache_table_entry *cache_tbl_entry_temp=0x0;
	for(struct cache_table_entry *i=cache_table_start;i;i=i->next)
	{
		if(i->path_hash==hash_value)
			cache_tbl_entry_temp=i;
	}
	if(!cache_tbl_entry_temp)
	{
		print("path not found in cache table\n");
		return -FILE_NOT_FOUND;
	}
	if(!cache_tbl_entry_temp->content.mnt_tbl_entry_ptr)
	{
		print("file open but not a mount point\n");
		return -FILE_NOT_MOUNTPOINT;
	}
	if(cache_tbl_entry_temp->refcount<1)
	{
		print("file is open by another application\n");
		return -RESOURCE_BUSY;
	}
	
	struct mount_table_entry* mnt_tbl_entry_temp=cache_tbl_entry_temp->content.mnt_tbl_entry_ptr;

	struct cache_table_entry *c_previous,*c_next;
	c_previous=cache_tbl_entry_temp->prev;
	c_next=cache_tbl_entry_temp->next;
	c_previous->next=c_next;
	c_next->prev=c_previous;
	heap_cream_free(cache_tbl_entry_temp);
	CACHE_TABLE_COUNT--;

	if(mnt_tbl_entry_temp->fs_bpb)
		heap_cream_free(mnt_tbl_entry_temp->fs_bpb);
	if(mnt_tbl_entry_temp->fs_root_node)
	{
		if(mnt_tbl_entry_temp->fs_root_node->fs_specific)
			heap_cream_free(mnt_tbl_entry_temp->fs_root_node->fs_specific);
		heap_cream_free(mnt_tbl_entry_temp->fs_root_node);
	}
	
	struct mount_table_entry *previous,*k_next;
	previous=mnt_tbl_entry_temp->prev;
	k_next=mnt_tbl_entry_temp->next;
	previous->next=k_next;
	k_next->prev=previous;

	heap_cream_free(mnt_tbl_entry_temp);
	return 0;
	
}
int  get_file_specific_fat16(struct vfs_node* parent,struct vfs_node* child,char* name)
{
	//FAT16_FILE_NOT_PRESENT_ERROR
	return 0;
}

void get_root_specific_fat16(struct vfs_node* node, struct partition* part)
{
		
}
