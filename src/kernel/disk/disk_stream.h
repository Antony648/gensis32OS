#ifndef DISKSTREAM_H
#define DISKSTREAM_H
#include "disk.h"
#include <stdint.h>

struct disk_stream
{
	uint32_t pos;
	struct disk* disk_struct;
};
struct disk_stream* init_disk_stream(uint32_t disk_id);
void disk_stream_seek(struct disk_stream* ds, uint32_t position);
void get_bytes_from_disk( struct disk_stream *ds, uint32_t no_bytes, uint8_t* ram_location);
void free_disk_stream(struct disk_stream* ds);

#endif
