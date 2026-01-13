#ifndef FAT16_H
#define FAT16_H 
#include "./virtual_file.h"
#include "partitions.h"
#include <stdint.h>

struct fat_16_root_dir_ent
{
	char name[8];
	char extension[3];
	uint8_t attr;
	uint8_t reserved;
	uint8_t creat_persc;
	uint16_t create_time;
	uint16_t create_date;
	uint16_t last_access_date;
	uint16_t high_word_cluster;
	uint16_t last_modified_time;
	uint16_t last_modified_date;
	uint16_t first_cluster;
	uint32_t file_size;

}__attribute__((packed));


int fat16_open(struct file_desc* fd);
int fat16_close(struct file_desc* file);
int fat16_read(struct file_desc* file,uint32_t offset,uint32_t len,char* buffer);
int fat16_write(struct file_desc* file,uint32_t offset,uint32_t len,char* buffer);


const struct file_operations fat16_fops=
{
	.open=fat16_open,
	.close=fat16_close,
	.read=fat16_read,
	.write=fat16_write
};
#endif
struct file_desc* get_root_fat16(struct partition* partition);
struct file_desc* get_root_child(struct partition* partition,char* name,char* extension);