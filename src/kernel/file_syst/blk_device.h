#ifndef  BLK_DEVICE_H
#define BLK_DEVICE_H
struct block_device
{
    struct partition* partition_ptr;
    //int (*read)(int )
    srurct block_device* next;
};
#endif
