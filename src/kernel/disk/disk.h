#ifndef DISK_H 
#define DISK_H

#include <stdint.h>


#define ATA_WAIT 100000
typedef uint32_t DISK_TYPE;

#define DISK_TYPE_REAL 0
#define DISK_TYPE_VIRTUAL 1
enum FILE_SYST_TYPE
{
	FS_UNKNOWN=0,
	FAT_12=0x01,
	FAT_16_L32=0x04,
	FAT_16_G32=0x06,
	FAT_16_LBA=0x0e,
	FAT_32_LBA=0x0c,
	FAT_32_CHS=0x0b,
	LINUX_NATIVE=0x83,
	
};

#include "../file_syst/partitions.h"
struct disk
{
	DISK_TYPE type;
	uint32_t ata_code;
	
	
	uint16_t base_data_port;
	uint16_t control_port;
	uint16_t command_status_port;
	uint16_t head;
	
	uint8_t drive_head; //0xa0 or 0xb0
	uint8_t is_master;
	
	uint32_t sect_size;
	uint32_t sect_count;
	struct partition* link_list;
	
	
};
void disk_search_and_init();
struct disk* get_disk(uint32_t index);
int read_disk_block(struct disk* disk_p,uint32_t lba, uint32_t total, void* buf);
void disk_debug_print();
//void destroy_disk_info();
#endif
