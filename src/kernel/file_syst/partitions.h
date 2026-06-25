#ifndef PARTITIONS_H
#define PARTITIONS_H
#include "../disk/disk.h"
#include <stdint.h>
void single_disk_scan(struct disk* disk_1);
void scan_part_all_disks();
void partition_debug();
//void seek_and_destroy();
#endif
