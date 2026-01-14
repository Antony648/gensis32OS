#ifndef FAT16_H
#define FAT16_H
#include "partitions.h"
#include "vfs.h"
#include <stdint.h>
#define  FAT16_FILE_NOT_PRESENT_ERROR 1
struct fat16_bpb
{
	uint16_t bytes_per_sect;
	uint8_t sect_per_clust;
	uint16_t reserved_sectors;
	uint8_t fat_count;
	uint16_t sectors_per_fat;
	uint16_t root_entry_count;

	uint32_t fat_lba;
	uint32_t root_lba;
	uint32_t data_lba;
};

void get_root_specific_fat16(struct vfs_node* node, struct partition* part);
int get_file_specific_fat16(struct vfs_node* parent,struct vfs_node* child,char* name);
int write_fat16(struct file* file_ptr,char* buffer,uint32_t size);
int read_fat16(struct file* file_ptr,char* buffer,uint32_t size);
void* parse_partition_fill_bpb_fat16(struct partition* );
struct file_system fat16_fs={
	.name="FAT16   ",
	.fopse={
		.get_root_specific=get_root_specific_fat16,
		.get_file_specific=get_file_specific_fat16,
		.vfs_write= write_fat16,
		.vfs_read=read_fat16,
		.parse_partition_fill_bpb=parse_partition_fill_bpb_fat16,
	}
};
#endif