#include "fat16.h"
#include "../disk/disk.h"
#include "../heap/heap_cream.h"
#include "vfs.h"
#include "../clock/clock.h"
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
#define F_NAME_LEN 13	//8 name+1 dot +3 extention+ null
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
uint32_t max(uint32_t val1,uint32_t val2)
{
	if(val1 > val2)
		return val1;
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

struct cache_table_entry* get_last_open(char* path,int* path_offset)
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
			{
				*path_offset=i;
				return k;
			}
			
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
uint16_t get_free_cluster(struct disk* disk,struct fat16_bpb* bpb,uint32_t start_fat,uint16_t cur_cluster)
{
	//start fat contains the start of fat sector with respect to disk 
	//start_fat=disk_start_sect+bpb->reserved_sector_start

	//load the fat_sectors one by one
		//in each fat_sectors,scan 2 bytes , till you find empty,
	//if we find one cluster empty, calculate new_cluster_number based on outer fat_sector count+inner offsetcount
	//set location of new_cluster_number in fat region as 0xffff
	//if cur_cluster < 0xfff8 ,then find its sector and offset fill its two bytes with new_cluster_number
	//return new_cluster_number
	uint16_t* single_sect =(uint16_t*)heap_cream_malloc(bpb->bytes_per_sect);
	uint16_t new_cluster_number=0xffff;
	uint32_t new_cluster_sector,cur_cluster_sect_no;
	uint32_t new_sect_offset=0xffffffff,size_of_fat_in_bytes=bpb->sectors_per_fat*bpb->bytes_per_sect;
	for(int i=0;i<bpb->sectors_per_fat;i++)
	{
		read_disk_block(disk, start_fat+i, 1, single_sect);
		for(int j=0;j<(bpb->bytes_per_sect/2);j++)
		{
			if(single_sect[j]==0x0000)
			{
				single_sect[j]=0xffff;
				new_cluster_sector=i;
				new_sect_offset=j;
				break;
			}
		}
		if(new_sect_offset>=(bpb->bytes_per_sect/2))
			continue;
		new_cluster_number=(i*(bpb->bytes_per_sect/2))+new_sect_offset;
		if(cur_cluster>=0xfff8 || cur_cluster< 0x0002)
		{
			write_disk_block(disk,start_fat+new_cluster_sector, 1, single_sect);
			cur_cluster_sect_no=0xffff;
			goto copy_fat_cleanup_return;
		}

		cur_cluster_sect_no=cur_cluster/(bpb->bytes_per_sect/2);
		if(cur_cluster_sect_no==new_cluster_sector)
		{
			single_sect[cur_cluster%(bpb->bytes_per_sect/2)]=new_cluster_number;
			write_disk_block(disk, start_fat+cur_cluster_sect_no, 1, single_sect);
			break;
		}
		write_disk_block(disk,start_fat+new_cluster_sector, 1, single_sect);
		read_disk_block(disk, start_fat+cur_cluster_sect_no, 1, single_sect);
		single_sect[cur_cluster%(bpb->bytes_per_sect/2)]=new_cluster_number;
		write_disk_block(disk, start_fat+cur_cluster_sect_no, 1, single_sect);
		break;
	}
copy_fat_cleanup_return:
	//making the changes we made in fat0 to other fats
	for(int i=1;i<bpb->fat_count;i++)
	{
		read_disk_block(disk, start_fat+(i*bpb->sectors_per_fat)+new_cluster_sector, 1, single_sect);
		single_sect[new_sect_offset]=0xffff;
		if(new_cluster_sector==cur_cluster_sect_no)
		{
			single_sect[cur_cluster%(bpb->bytes_per_sect/2)]=new_cluster_number;	
			write_disk_block(disk, start_fat+(i*bpb->sectors_per_fat)+new_cluster_sector, 1, single_sect);
			continue;
		}
		write_disk_block(disk, start_fat+(i*bpb->sectors_per_fat)+new_cluster_sector, 1, single_sect);

		//for handling where our init address was 0xffff or 0x0002
		if(cur_cluster_sect_no>=0xfff8)
			continue;
		read_disk_block(disk, start_fat+(i*bpb->sectors_per_fat)+cur_cluster_sect_no, 1, single_sect);
		single_sect[cur_cluster%(bpb->bytes_per_sect/2)]=new_cluster_number;
		write_disk_block(disk, start_fat+(i*bpb->sectors_per_fat)+cur_cluster_sect_no, 1, single_sect);

	}


	heap_cream_free(single_sect);
	return new_cluster_number;
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
	if(!size || !file_ptr)
		return 0;
	uint32_t buffer_index=0;
	uint32_t data_sec_clust_start=file_ptr->vfs_node_ptr->fs_specific;
	struct mount_table_entry* mnt_tble=file_ptr->vfs_node_ptr->origin_mount_point;
	struct fat16_bpb * bpb=(struct fat16_bpb*)mnt_tble->fs_bpb;

	uint32_t cluster_count= file_ptr->offset/(bpb->bytes_per_sect*bpb->sect_per_clust);  //tells how many clusters should we hop using FAT
	uint32_t offset_in_cluster= file_ptr->offset%(bpb->bytes_per_sect*bpb->sect_per_clust); 
	uint32_t start_sect_part=mnt_tble->mnt_part->start_sect;
	uint16_t cur_cluster=(uint16_t)file_ptr->vfs_node_ptr->fs_specific;
	struct disk* disk_cur=mnt_tble->mnt_part->f_disk;
	if((cur_cluster >=0xfff8 )|| (cur_cluster< 0x0002))
	{
		//code for finding free cluster and adding it to cur_cluster and setting the location in fat of that 
		//cluster to 0xffff
		cur_cluster=get_free_cluster(disk_cur, bpb, start_sect_part+bpb->reserved_sectors, cur_cluster);
		file_ptr->vfs_node_ptr->fs_specific=cur_cluster;
		file_ptr->vfs_node_ptr->dirty_bit=1;
	}
	
	uint8_t* single_cluster=(uint8_t*)heap_cream_malloc((size_t)(bpb->bytes_per_sect));
	uint32_t temp=bpb->reserved_sectors+bpb->sectors_per_fat*bpb->fat_count;
	temp+=(bpb->root_entry_count*32+(bpb->bytes_per_sect-1))/bpb->bytes_per_sect;
	uint8_t* cluster_temp=heap_cream_malloc(bpb->sect_per_clust*bpb->bytes_per_sect);


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

			//check cur_cluster if 0xffff or 0xfff8, if yes find new empty cluster
			if((cur_cluster >=0xfff8 )|| (cur_cluster< 0x0002))
			{
				cur_cluster=get_free_cluster(disk_cur, bpb, start_sect_part+bpb->reserved_sectors, cur_cluster);
				file_ptr->vfs_node_ptr->dirty_bit=1;
			}

		}
		//this loop deals with writing,
		//the underlying disk_read can only read as a sectior 512 bytes,so we should
		//make that adjustment here as to how many sectors of size 512 make up a partition specific sector

		//loop logic:
		//dump whole sector to cluster_temp,
		//target_size=min(size,(size_of_cluster-offset)) 
		//copy bytes from offset_in_cluster to the buffer starting from buffer_index 
		//target_size bytes,update buffer_index,

		//clear cluster_temp,use it to load the dir ent, find the next sector to read
		//if 0xff, end of file , if another sector, set it as start cluster and offset=0
		
		for(int q=0;q<bpb->sect_per_clust;q++)
		{
			uint32_t cluster_temp_offset=q*bpb->bytes_per_sect;
			read_disk_block(disk_cur, start_sect_part+temp+((cur_cluster-2)*bpb->sect_per_clust)+q, (bpb->bytes_per_sect/SECTOR_SIZE_DISK_GENERAL_BYTES),&(cluster_temp[cluster_temp_offset]));
		}
		//the whole cluster is loaded into the cluster_temp
		uint32_t target_size=min(size-buffer_index,((bpb->bytes_per_sect*bpb->sect_per_clust)-offset_in_cluster));
		memcpy(&(cluster_temp[offset_in_cluster]),&(buffer[buffer_index]),target_size);
		
		for(int q=0;q<bpb->sect_per_clust;q++)
		{
			uint32_t cluster_temp_offset=q*bpb->bytes_per_sect;
			write_disk_block(disk_cur, start_sect_part+temp+((cur_cluster-2)*bpb->sect_per_clust)+q, (bpb->bytes_per_sect/SECTOR_SIZE_DISK_GENERAL_BYTES),&(cluster_temp[cluster_temp_offset]));
		}

		buffer_index+=target_size;
		file_ptr->offset+=target_size;	// updating offset in file
		file_ptr->vfs_node_ptr->size=max(file_ptr->vfs_node_ptr->size,buffer_index);

		if(buffer_index>= size)
			goto cleanup_return;
		cluster_count=1;offset_in_cluster=0;
		//load next cluster 
		

	}



	cleanup_return:
		heap_cream_free(single_cluster);
		heap_cream_free(cluster_temp);
		return buffer_index;
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
	uint32_t temp=bpb->reserved_sectors+bpb->sectors_per_fat*bpb->fat_count;
	temp+=(bpb->root_entry_count*32+(bpb->bytes_per_sect-1))/bpb->bytes_per_sect;
	
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
cleanup_return:
	heap_cream_free(single_cluster);
	heap_cream_free(cluster_temp);

	file_ptr->offset+=buffer_index;
	return buffer_index;

}
struct cache_table_entry* search_cache_table(uint64_t hash_val)
{
	struct cache_table_entry *temp=cache_table_start;
	while (temp)
	{
		if(temp->path_hash==hash_val)
			return temp;
	}
	return NULL;
}
uint8_t get_fs_specific_path_last_dir(char* path,uint16_t *fs_specific)
{
	//return code:
	//0x00 failure
	//0x01 sending fs_specific of the dir just above the file
	//0x02 file already open in cachetable
	int len=strlen((const char*)path);
	if(path[len-1]=='/')	//excluding the last / in the search 
		len-=2;
	else
	 	len-=1;
	struct cache_table_entry* ct_entry=search_cache_table(generate_hast_start_end(path, 0, len));
	if(ct_entry)
		{
			*fs_specific=(uint16_t)get_fs_specific_cache_table(ct_entry); 
			return  0x02;
		}
	int i=len;
	int success=1;
	while(1)
	{
		
		while(i>=0 && path[i]!='/')
			i--;
		if(i<=0)
			goto failure;
		i--;
		//generate hash for path excluding last word
		uint64_t hash_val=generate_hast_start_end(path, 0, i);
		//check if the path is open in cache table
		ct_entry=search_cache_table(hash_val);
		if(ct_entry)
			break;
		success=0;	//if it hits alteast once then, we donot have cache table entry for the dir just above
		//the target dir

	}
	if(success)
	{
		*fs_specific=(uint16_t)get_fs_specific_cache_table(ct_entry);
		return 0x00;
	}
	//we have work to do
	//i contains last path except the /, check if it is a directory,root
	//get immediate child , single dir or file,from path
	//use fs_specific of ct_entry,load clusters one by one, try to find the child
	//or 
	// open the immdiate child , read its contents...
	//close
	//check for dir_entry of the next child that we got from path
	//if found , update
	return 0x00;
failure:
	return 0x00;
}
int set_fname(char* path,char *f_name,int start ,size_t size)
{
	for(uint32_t i=0;i<size;i++)
		f_name[i]=0;
	int i,j;
	for ( i=start;path[i]!='/' && path[i];i++);
	for (j=start;j<i;j++,i++)
		f_name[j] =path[i];
	f_name[j]=0;
	return  i-1;	//index of last element

}
uint32_t get_cluster_size(struct vfs_node* node)
{
	//return size of cluster in bytes
	struct fat16_bpb * bpb=(struct fat16_bpb *)node->origin_mount_point->fs_bpb;
	return (bpb->bytes_per_sect*bpb->sect_per_clust);
}
char* read_cluster_find_match(struct cache_table_entry* ct,char* f_name,char* fill_val)
{
	char* ret_val=0x0;
	struct vfs_node* node;
	uint32_t cluster_size=0;
	char* buffer=NULL;
	char* target_name=NULL;
	switch(ct->content_type)
	{
		case CTE_FILE:
			return 0x0;
		case CTE_MOUNT_PNT:
		case CTE_ROOT:
			node =ct->content.mnt_tbl_entry_ptr->fs_root_node;
			break;
		case CTE_DIR:
			node=ct->content.vfs_node_ptr;
			break;
		default:
			return 0x0;
	}
	//create a file pointer for read
	struct file file_ptr;
	file_ptr.vfs_node_ptr=node;
	file_ptr.offset=0;
	file_ptr.flags=0x1;	//read
	cluster_size=get_cluster_size(node);	//cluster size in bytes

	buffer=heap_cream_malloc(cluster_size);
	if(!buffer)
		return 0x0;
	for(int j=0;j<((int)(node->size/cluster_size));j++)
	{
		//loop till 
		if(!read_fat16(&file_ptr, buffer, cluster_size))
			break;
		//loop terminates if read_fat16 returns 0 , or 0 bytes read 
		target_name=buffer;	//set target_name to buffer so that it points to start
		for(int i=0;i<(cluster_size/FAT16_DIRENT_SIZE);i++)
		{
			//check for a file with f_name in buffer
			if(!strncmp(target_name, f_name, FILE_NAME_LEN_MAX))
			{
				memcpy(fill_val, target_name, FAT16_DIRENT_SIZE);
				ret_val=target_name;goto exit;
			}
			target_name+=FAT16_DIRENT_SIZE;
		}
	}

exit:
	if(buffer)
		heap_cream_free(buffer);
	return ret_val;
}
struct mount_table_entry* get_origing_mnt_tbl_ent(struct cache_table_entry* ct)
{
	struct vfs_node* node=0;
	switch(ct->content_type)
	{
		case CTE_FILE:
			return NULL;
		case CTE_MOUNT_PNT:
		case CTE_ROOT:
			node =ct->content.mnt_tbl_entry_ptr->fs_root_node;
			break;
		case CTE_DIR:
			node=ct->content.vfs_node_ptr;
			break;
		default:
			return NULL;
	}
	return node->origin_mount_point;
	
}
struct cache_table_entry* fat16_sub_open_file(char* dir_ent,char*path,int offset,struct mount_table_entry* origin)
{
	//create a cache_table_entry
	struct cache_table_entry* ct=heap_cream_malloc(sizeof(struct cache_table_entry));
	if(!ct)
		return NULL;
	ct->path_hash=generate_hast_start_end(path, 0, offset);
	if((dir_ent[11])&0x10)
		ct->content_type=CTE_DIR;
	else
	 	ct->content_type=CTE_FILE;

	//allocate a vfs_node
	struct vfs_node *node=heap_cream_malloc(sizeof(struct vfs_node));
	if(!node)
		goto fail;

	ct->content.vfs_node_ptr=node;
	ct->refcount=1;
	ct->flags=file_read;
	if(!(dir_ent[11]&0x01))
		ct->flags|=file_write;
	ct->next=NULL;
	cache_table_last->next=ct;
	ct->prev=cache_table_last;
	cache_table_last=ct;

	//modify and link vfs to ct
	node->fs_specific= *((uint16_t*)&dir_ent[26]);
	node->size=*((uint32_t*)&dir_ent[28]);
	node->content.ct=ct;
	//perform origin scan to find actual origin instead of passed origin;may be mount point
	node->origin_mount_point=origin;
	node->node_id=generate_vfs_node_id();
	node->mode=ct->flags;
	//set node access time and creation time
	return ct;
fail:
	if(ct)
		heap_cream_free(ct);
	if(node)
		heap_cream_free(node);
	return NULL;
}
int fat16_sub_close_file(struct cache_table_entry* ct)
{
	//cannot close mount point
	//check refcount
	if(!ct)
		return -NULL_ARG;
	if(ct->refcount>1)
		return -MORE_THAN_ONE_REFCOUNT;
	if(ct->content_type == CTE_MOUNT_PNT)
		return -TRYING_TO_CLOSE_MOUNT_PNT;

	struct vfs_node* node=ct->content.vfs_node_ptr;
	struct cache_table_entry* temp=ct->next;
	ct->prev->next=temp;
	temp->prev=ct->prev;
	//destroy ct
	heap_cream_free(ct);
	//destroy node
	if(node)
		heap_cream_free(node);
	return 0;
}
int create_file_fat16(char* path,uint8_t type)
{
	int rtn_val,start=0,cur_name_end;
	int path_last=strlen(path);
	if(path[path_last-1]=='/')
		path_last--;
	char f_name[F_NAME_LEN];
	char dir_ent[FAT16_DIRENT_SIZE];
	char* dir_ent_location=NULL;
	char* dir_ent_prev=NULL;
 	bool is_first=true;
 	
	/*
	set start=0;
	use get_last_open with &start as second param , 
	the full path will contian root or /root as start or any mount point
	and it will be open so we will not get 0 as start in an expected situvation
	cur_file= file retured from get_last_open
	check if cur_file is not a directory :
		rtn_val=-NON_EXISTENT_PATH;goto exit;(return -NON_EXISTENT_PATH)
		 */
	struct cache_table_entry* cur_file=get_last_open(path, &start);
	if(cur_file->content_type==CTE_FILE)
	{
		//okay to be dir, root, mount point as all of this are directories
		rtn_val=-NON_EXISTENT_PATH;goto exit;
	}
		 /*
	iteration:
		f_name= immediate descendent file name from path start,cur_name_end holds end of name in path
		for all clusters till match found:
			read the clusters  cur_file, check for the f_name (load 32 byte dir entries )
			if match found:
				check if cur_name_end ==end of path:
					it means files already exists;rtn_val=-FILE_ALREADY_EXISTS;goto exit;(return -FILE_ALREADY_EXISTS)
				check if file is not dir:
					it means we cannot have subdir yet path incomplete
					rtn_val=-NON_EXISTENT_PATH;goto exit;(return -NON_EXISTENT_PATH)
				if not first iteration:
						ntermediate parent we opend), destroy file_ptr and cache_table_entry
				open that file, create entry in cache_table entry and create file pointer 
				set that to cur_file
				update path "start" to include now opened file
				goto iteration
		if match not found: 
			check if cur_name_end is the end of path(if the file we want is the last file):
				create a file with f_name in the cur_file as per type give it one cluster
				modify the parent*(cur_file) with one more dir_ent of the new f_name
				rtn_val=0;goto exit;(return 0)
			else:
				create a file with f_name under cur_file as dir and give it one cluster 
				modify the parent(cur_file) with one more dir_ent of the new f_name
				close cur_file (the intermediate parent we opend), destroy file_ptr and cache_table_entry
				open that file, create entry in cache_table entry and create file pointer 
				set that to cur_file
				update path "start" to include now opened file
						
	 */ 
	
	 while(1)
	 {
		cur_name_end=set_fname(path, f_name, start, F_NAME_LEN);
		//cur_name_end++;
		dir_ent_prev=dir_ent_location;
		dir_ent_location=read_cluster_find_match(cur_file,f_name,dir_ent);
		if(dir_ent_location)
		{
			if(cur_name_end ==path_last)
			{
				rtn_val=-PATH_ALREADY_EXISTS;goto exit;
			}
			else if(!((dir_ent[11])&0x10))	 //if the bit 4 is not  set then not dir
			{
				rtn_val=-NON_EXISTENT_PATH;goto exit;
			}
			else {
				struct mount_table_entry* origin=get_origing_mnt_tbl_ent(cur_file);
				if(is_first)
					is_first=false;
				else
				{
					//close the temp open file
					fat16_sub_close_file(cur_file);
				}
				//open that file
				cur_file=fat16_sub_open_file(dir_ent,path,cur_name_end-1,origin);

				//update start to cur_name_end; if it does not cause trouble
				if(path[cur_name_end] && path[cur_name_end+1])	//concept of cur_name being end alredy checked
					start=cur_name_end+1;
			}
		}
		else 
		{
			//once created all subfolders and files should be created ;no need ot scan or find
			struct disk* disk;struct fat16_bpb *bpb;struct mount_table_entry *origin;
			struct file temp_file;
			if(dir_ent_prev)
				temp_file.offset=*((uint16_t*)(&dir_ent_prev[28]));//puts offset to end
			do{
				//check the write permission of the cur_file
				//create one dir ent in the current file, fill the required details
				strncpy(dir_ent,f_name,FILE_NAME_LEN_MAX);
				strncpy(&(dir_ent[FILE_NAME_LEN_MAX]),&(f_name[FILE_NAME_LEN_MAX+1]),(F_NAME_LEN-FILE_NAME_LEN_MAX)); 
				uint16_t temp=get_date_fat16();
				*((uint16_t*)(&dir_ent[24]))=temp;
				*((uint16_t*)(&dir_ent[18]))=temp;
				*((uint16_t*)(&dir_ent[16]))=temp;
				temp=get_time_fat16();
				*((uint16_t*)(&dir_ent[14]))=temp;
				*((uint16_t*)(&dir_ent[22]))=temp;
				dir_ent[20]=0;dir_ent[21]=0;
				origin=get_origing_mnt_tbl_ent(cur_file);
				if(!origin)
				{
					rtn_val= -COULD_NOT_FIND_CACHE_TABLE_ENTRY;
					goto exit;
				}
				disk=origin->mnt_part->f_disk;
				bpb=(struct fat16_bpb*)origin->fs_bpb;
				uint32_t start_sect=origin->mnt_part->start_sect;
				temp=get_free_cluster(disk, bpb, start_sect, 0xfff8);
				if(temp>= 0xfff8 || temp< 0x2)
				{
					rtn_val=-FAILED_TO_GET_FREE_CLUSTER;
					goto exit;
				}
				*((uint16_t*)(&dir_ent[26]))=temp;
				*((uint16_t*)(&dir_ent[28]))=0;
				//get one empty cluster from fat table set the status in FAT, if fail undo previoius exit
				//struct file temp_file;temp_file.offset=*((uint16_t*)(&dir_ent_prev[28]));//puts offset to end
				//the above line is put to top so that it only executes during initial run
				//because once inside this creation loop we donot the size of parent size to append as 
				//we know the size is zero because inside loop parent is freshly created
				temp_file.flags=file_write;
				if(cur_file->content_type==CTE_MOUNT_PNT)
					temp_file.vfs_node_ptr=cur_file->content.mnt_tbl_entry_ptr->fs_root_node;
				else
					temp_file.vfs_node_ptr=cur_file->content.vfs_node_ptr;
				if(write_fat16(&temp_file, dir_ent, FAT16_DIRENT_SIZE)<0)
				{
					//some error ,undo the Fat table write; 
					//YET TO IMPLEMENT
					rtn_val=-FAILED_TO_WRITE_TO_PARENT;goto exit;
				}
				//assign the empty cluster found to field in dir
				if(cur_name_end == path_last)
				{
					//file created ,set return value;
					//iteratively close file that has been opened yet to implement!
					fat16_sub_close_file(cur_file);
					rtn_val=0;goto exit;
				}
				else {
					//close the parent cur_file and go to newly created child
					fat16_sub_close_file(cur_file);
					cur_file=fat16_sub_open_file(dir_ent,path,cur_name_end-1,origin);
					if(path[cur_name_end] && path[cur_name_end+1])
						start=cur_name_end+1;
				}
				cur_name_end=set_fname(path, f_name, start, F_NAME_LEN);
				temp_file.offset=0;
			}while(cur_name_end<= path_last);
			goto exit;
		}
	 }
exit:
//unallocate all resources
//check if last temp file was deleted if not delete
/*if	not first iteration:
		delete cur_file file_ptr and cache table entry*/
	if(!is_first)
	{
		if(cur_file)
		fat16_sub_close_file(cur_file);
	}
	return rtn_val;
}
int delete_file_fat16(char* path)
{
	int rtn_val=0;
	/*
	set start=0;
	use get_last_open with &start as second param , 
	the full path will contian root or /root as start or any mount point
	and it will be open so we will not get 0 as start in an expected situvation
	cur_file= file retured from get_last_open
	check if cur_file is not a directory :
		rtn_val=-NON_EXISTENT_PATH;goto exit;(return -NON_EXISTENT_PATH)
	iteration:
		fname=immediate descendent file name from path start,cur_name_end holds end of name in path
		for each cluster in said file:
			if f_name found:
				if cur_name_end  ==end of path:
					if f_name is not dir || (is dir but empty):
						free all clusters of f_name; remove dir_ent of f_name from cur_file(immediate parent)
						rtn_val=0;goto exit; (return 0)
					else:	
						rtn_val=-DELETE_NON_EMPTY_DIR_ERROR;goto exit;
				else:
					if f_name is not dir :
						rtn_val=-NON_EXISTENT_PATH;goto exit;
					else:
						if	not first iteration:
							delete cur_file file_ptr and cache table entry
						open f_name, file_ptr and cache table entry
						set cur_file to f_name, update start to include f_name 
						continue;(goto iteration)
			else:
				rtn_val=-FILE_NOT_FOUND; goto exit;(return -FILE_NOT_FOUND)
					

	*/	
exit:
	//unallocate all resources
	//check if last temp file was deleted if not delete
	/*if	not first iteration:
		delete cur_file file_ptr and cache table entry*/
	return rtn_val;
}