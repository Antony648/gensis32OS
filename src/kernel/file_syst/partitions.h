#ifndef PARTITIONS_H
#define PARTITIONS_H
#include "../disk/disk.h"
#include <stdint.h>
struct partition
{
	struct disk* f_disk;
	enum FILE_SYST_TYPE fs_type;
	uint32_t start_sect;
	uint32_t sect_num;
	uint8_t is_bootable;
	struct partition* next;
};
void single_disk_scan(struct disk* disk_1);
void scan_part_all_disks();
void partition_debug();
void seek_and_destroy();
#endif
