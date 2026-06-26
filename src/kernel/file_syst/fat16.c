#include "fat16.h"
#include "../disk/disk.h"
#include "../heap/heap_cream.h"
#include "vfs.h"
#include <stddef.h>
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
uint32_t min(uint32_t val1,uint32_t val2)
{
	if(val1 < val2)
		return  val1;
	return val2;
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

int mount_fat16(struct partition* part,char* path,struct vfs_node*r_node)
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
	//struct vfs_node* r_node=NULL;
	//r_node=(struct vfs_node*)heap_cream_malloc(sizeof(struct vfs_node));
	if(!r_node)
		goto fail;

	mnt_tbl_e->fs_root_node=r_node;
	mnt_tbl_e->mnt_part=part;
	mnt_tbl_e->fs_bpb=(void*)bpb;

	//node
	r_node->node_id=generate_vfs_node_id();
	r_node->content.mte=mnt_tbl_e;
	uint32_t root_dir_count=bpb->data_lba-bpb->root_lba;

	r_node->size=bpb->bytes_per_sect* root_dir_count;
	r_node->fs_specific=(uint32_t)(bpb->reserved_sectors+(uint16_t)bpb->fat_count*bpb->sectors_per_fat);

	//heap_cream_free(bpb);
	//ct
	i=heap_cream_malloc(sizeof(struct cache_table_entry));
	i->refcount=1;
	i->content_type=CTE_MOUNT_PNT;
	i->content.mnt_tbl_entry_ptr=mnt_tbl_e;	//cachetableentry points to  mounttableentry which has vfsnodeptr
	i->path_hash=hash_value;

	CACHE_TABLE_COUNT++;
	mnt_tbl_e->ct_table_entry=i;

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
		heap_cream_free(mnt_tbl_entry_temp->fs_root_node);
	
	
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
	//get data section  start cluster of the file
	//get the data section of the parent node from fs_specific
	//dump sector_per_cluster count from the data section of parent
	//check for the given file, if not found in given cluster
	//go to FAT and load the next cluster, repeat the process till you find file
	//if file not found return file not found or file not present
	//if files entry found , find its start cluster in datasection, 
	//is the file the last file in the path or the required file? 
	//if yes create cache table entry, assign values to node, and return success
	//if no , then repeat from step 1 for the subdirectory
	//FAT16_FILE_NOT_PRESENT_ERROR

	return 0;
}
struct cache_table_entry* is_present_in_cache_table(int mode,uint64_t hash, char* path)
{
	//mode 1 : hash, mode2: cache
	struct cache_table_entry *cache_tbl_entry_temp=0x0;
	switch(mode)
	{
		case 2:
			hash=generate_hash(path);
		case 1:
			for(struct cache_table_entry *i=cache_table_start;i;i=i->next)
			{
				if(i->path_hash==hash)
					cache_tbl_entry_temp=i;
			}
			break;
		default: print("fat16.c:illegal call in is_present_in_cache_table\n");return 0x00;
	}
	return cache_tbl_entry_temp;
}

struct cache_table_entry* get_last_open(char* path)
{
	//break the path using delimeter from reverse
	//generate hash for whole path(exclude delimiter at end) , see if it is aldready open
	//if no, exclude the last word and the delimiter before it and generate hash and compare
	//repeat above step till you hit a value in cache table.. after finding the value, get cache_table_entry
	
	int len=strlen(path);
	uint32_t end=-1;
	for(int i=len-1;i>=0;i--)
	{
		if(path[i]=='/')
		{
			i--;
			uint64_t hash=generate_hast_start_end(path, 0, i);
			struct cache_table_entry* k=is_present_in_cache_table(1,hash,NULL);
			if(k)
				return k;
			
		}
		
	}
	return (struct cache_table_entry*)0x0;
}
uint32_t get_root_specific_fat16(struct vfs_node* node, struct partition* part)
{
	 struct fat16_bpb* bpb=parse_partition_fill_bpb_fat16(part);
	 //we have to caclulate start of root section
	return (uint32_t)(bpb->reserved_sectors+(uint16_t)bpb->fat_count*bpb->sectors_per_fat);
	 
}
int write_fat16(struct file* file_ptr,char* buffer,uint32_t size)
{
	//use the file to get to the node and get the fs_specific(start of cluster in data section of file)
	//find the offset value form the file struct, compute the bytes to know what cluster to read data 
	//from , if the bytes is less than a cluster, get cluster zero, of clusternumber=startcluster+(bytesoffset/bytespercluster)
	//and offset in that cluster by offsetinclusterbytes=(bytesoffset%bytespercluster)
	//now based on the value of clusternumber , go to FAT and identify next or following clusters till ,for 
	//clusternumber of times, ie:if clusternumber=0, go to fat , return the value after 0 jumps, if value is 2
	//go to fat, (currently at cluster 0),jump1, go to the next cluster, jump2 , now we are at data start of 
	//requried cluster, after you have the actual cluster start offset in data section
	//copy that entire cluster to a local buffer, now in the offsetinclusterbytes, start writing the contents 
	//in that sector now write the entire cluster back into the disk, check if it goes out of bound with the cluster
	//if it goes out of bound, check for the next cluster , if present, load it write the remaining contents
	//and continue the process

	//if next cluster not present,modify the fat section, find a free cluster modify current cluster end value
	//(0xffff or 0xfff8)with new found cluster, and set the new cluster with 0xffff in the fat table, 
	//memset the buffer in hand write the remaining contnets and write it back to the disk

	//continue above till the buffer is empty
	return 0;
}
int read_fat16(struct file* file_ptr,char* buffer,uint32_t size)
{
	if(!size || !file_ptr)
		return 0;
	//use the file to get node, and get fs_specific(start of cluster in data section of file)
	//now compute the clusternumber, clusternumber=startcluster+(bytesoffset/bytespercluster)
	//now compute the offset, offsetinclusterbytes=(bytesoffset%bytespercluster)
	//load the clusternumber , a single cluster into local memory, read from offsetinclusterbytes
	//till value of size, if size outside cluster bounds, 
	//use fat section and locate next cluster and load it to local memory, read remaining
	//repeat process till size is reached
	//if next cluster is not present, return end of file after reading the last cluster
	
	uint32_t data_sec_clust_start=file_ptr->vfs_node_ptr->fs_specific;
	struct mount_table_entry* mnt_tble=file_ptr->vfs_node_ptr->origin_mount_point;
	struct fat16_bpb * bpb=(struct fat16_bpb*)mnt_tble->fs_bpb;

	uint32_t cluster_count= file_ptr->offset/(bpb->bytes_per_sect*bpb->sect_per_clust);  //tells how many clusters should we hop using FAT
	uint32_t offset_in_cluster= file_ptr->offset%(bpb->bytes_per_sect*bpb->sect_per_clust); 
	uint32_t start_sect_part=mnt_tble->mnt_part->start_sect;
	//tells us the offset in the given cluster

	
	uint16_t cur_cluster=(uint16_t)file_ptr->vfs_node_ptr->fs_specific;
	if((cur_cluster >=0xfff8 )|| (cur_cluster< 0x0002))
				return 0;
	uint32_t buffer_index=0;
	uint8_t* single_cluster=(uint8_t*)heap_cream_malloc((size_t)(bpb->bytes_per_sect));

	
	struct disk* disk_cur=mnt_tble->mnt_part->f_disk;
	
	
	//our current_cluster has the cluster we need to start reading the file after offset computation

	uint8_t* cluster_temp=heap_cream_malloc((size_t)(bpb->bytes_per_sect*bpb->sect_per_clust));
	//we have found the start cluster to read as per offset and also offset_in_cluster
	//int size=size_to_read;	//converting to signed integer for next operations

	while(1)
	{
		for(uint32_t i=0;i<cluster_count;i++)
		{
			
			uint64_t actual_byte_offset=cur_cluster*2;
			uint16_t byte_offset=(uint16_t)actual_byte_offset;

			//compute which fat cluster will have the required cluster number based on partition
			//now add it with start sector of partition and load it
			//find the location where offset is stored inside said loaded cluster
			//calculate the entry of the cluster to find the cur cluster

			uint32_t sector_to_load=(bpb->reserved_sectors)+(actual_byte_offset/bpb->bytes_per_sect);
			read_disk_block(disk_cur, start_sect_part+sector_to_load, (bpb->bytes_per_sect/SECTOR_SIZE_DISK_GENERAL_BYTES),single_cluster);
			cur_cluster=*(uint16_t*)&single_cluster[actual_byte_offset%bpb->bytes_per_sect];

			//check cur_cluster if 0xff or 0xf8 return end of file
			if((cur_cluster >=0xfff8 )|| (cur_cluster< 0x0002))
				goto cleanup_return;
			

		}
		//this loop deals with reading,
		//the underlying disk_read can only read as a sectior 512 bytes,so we should
		//make that adjustment here as to how many sectors of size 512 make up a partition specific sector

		//loop logic:
		//dump whole sector to cluster_temp,
		//target_size=min(size,(size_of_cluster-offset)) 
		//copy bytes from offset_in_cluster to the buffer starting from buffer_index 
		//target_size bytes,update buffer_index,

		//clear cluster_temp,use it to load the dir ent, find the next sector to read
		//if 0xff, end of file , if another sector, set it as start cluster and offset=0
		uint32_t temp=bpb->reserved_sectors+bpb->sectors_per_fat*bpb->fat_count;
		temp+=(bpb->root_entry_count*32+(bpb->bytes_per_sect-1))/bpb->bytes_per_sect;
		for(int q=0;q<bpb->sect_per_clust;q++)
		{
			uint32_t cluster_temp_offset=q*bpb->bytes_per_sect;
			read_disk_block(disk_cur, start_sect_part+temp+((cur_cluster-2)*bpb->sect_per_clust)+q, (bpb->bytes_per_sect/SECTOR_SIZE_DISK_GENERAL_BYTES),&(cluster_temp[cluster_temp_offset]));
		}
		//the whole cluster is loaded into the cluster_temp
		uint32_t target_size=min(size-buffer_index,((bpb->bytes_per_sect*bpb->sect_per_clust)-offset_in_cluster));
		memcpy(&(buffer[buffer_index]),&(cluster_temp[offset_in_cluster]),target_size);
		buffer_index+=target_size;

		if(buffer_index>= size)
			goto cleanup_return;
		cluster_count=1;offset_in_cluster=0;
		//load next cluster 
		

	}
	heap_cream_free(cluster_temp);
cleanup_return:
	heap_cream_free(single_cluster);
	heap_cream_free(cluster_temp);

	file_ptr->offset+=buffer_index;
	return buffer_index;

}
int create_file_fat16(char* path)
{
	return 0;
}
int delete_file_fat16(char* path)
{
	return 0;
}