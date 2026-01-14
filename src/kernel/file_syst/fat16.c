#include "fat16.h"
#include "../disk/disk.h"
#include "../heap/heap_cream.h"
#include <stdint.h>

static inline  uint16_t get_2_bytes(uint8_t* ptr)
{
	//we require this because of endianess
	return (uint16_t)((ptr[1]<<8)|ptr[0]);
}
static inline uint32_t get_4_byte(uint8_t* ptr)
{
	return  (uint32_t)((ptr[2]<<24)|(ptr[1]<<16)|(ptr[0])<<8|ptr[0]);
}
void* parse_partition_fill_bpb_fat16(struct partition* part )
{
	//
	uint8_t* sect_0_buf=heap_cream_malloc(512);
	if(read_disk_block(part->f_disk, part->start_sect, 1, sect_0_buf)<0)
	{
		heap_cream_free(sect_0_buf); return NULL;
	}
	struct fat16_bpb* bpb_ptr=(struct fat16_bpb*)heap_cream_malloc(sizeof(struct fat16_bpb));
	bpb_ptr->bytes_per_sect=get_2_bytes(&sect_0_buf[0x0b]);
	bpb_ptr->sect_per_clust=sect_0_buf[0x0d];
	bpb_ptr->reserved_sectors=get_2_bytes(&sect_0_buf[0xe]);
	bpb_ptr->fat_count=sect_0_buf[0x10];
	bpb_ptr->root_entry_count=get_2_bytes(&sect_0_buf[0x11]);
	bpb_ptr->sectors_per_fat=get_2_bytes(&sect_0_buf[0x16]);
	bpb_ptr->fat_lba=part->start_sect+ bpb_ptr->reserved_sectors;
	bpb_ptr->root_lba=bpb_ptr->fat_lba+(bpb_ptr->fat_count* bpb_ptr->sectors_per_fat);
	uint32_t root_dir_count;

	root_dir_count=((bpb_ptr->root_entry_count * 32)+(bpb_ptr->bytes_per_sect-1))/bpb_ptr->bytes_per_sect;
	//(A+B-1)/B; A/B in rounded div format

	bpb_ptr->data_lba=bpb_ptr->root_lba+root_dir_count;
	heap_cream_free(sect_0_buf);
	return bpb_ptr;
}

int  get_file_specific_fat16(struct vfs_node* parent,struct vfs_node* child,char* name)
{
	//FAT16_FILE_NOT_PRESENT_ERROR
	return 0;
}

void get_root_specific_fat16(struct vfs_node* node, struct partition* part)
{
		
}