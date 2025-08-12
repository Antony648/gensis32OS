#ifndef DISK_H
#define DISK_H

#include <stdint.h>
int read_sect_disk(int lba, int total, void* buf);
#endif
