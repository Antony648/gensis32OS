#ifndef DISK_H 
#define DISK_H

#include <stdint.h>


typedef uint32_t DISK_TYPE;

#define DISK_TYPE_REAL 0
#define DISK_TYPE_VIRTUAL 1
enum FILE_SYST_TYPE
{
	FS_UNKOWN=0,
	FAT_16,
	FAT_32,
	EXT_2
};
struct partition
{
	struct disk* f_disk;
	enum FILE_SYST_TYPE fs_type;
	uint32_t start_sect;
	uint32_t sect_num;
};
struct disk
{
	DISK_TYPE type;
	uint32_t size;
	
};
void disk_search_and_init();
struct disk* get_disk(uint32_t index);
int read_disk_block(struct disk* disk_p,uint32_t lba, uint32_t total, void* buf);
#endif
