#ifndef OSCONFIG_H
#define OSCONFIG_H

#define OSNAME "genesis32"
#define TOTAL_INTERRUPTS 		512
#define KERNEL_CODE_SELECTOR 	0x08
#define KERNEL_DATA_SELECTOR 	0X10
#define	KHEAP_SIZE_BYTES		0x6400000
#define KHEAP_START 			0x1000000
#define KHEAP_COUNT				25600  //no of 4kb pages possible in 100 mb 
#define KHEAP_TABLE				0x7e00
#define KHEAP_BLOCK_SIZE		4096
#define KHEAP_BLOCK_SHIFT		12
#define SECTOR_SIZE				512
#define MAX_OPEN_FILE_COUNT		50
#define FILE_NAME_LEN_MAX 		20
#define FS_MOUNTS_MAX			8
#define DISK_SUPPORT_MAX		5

#define FS_DEPEND_FUNC			4
#define VFS_GLOBAL_FAT16_ID		0
#define VFS_GLOBAL_FAT32_ID		1 //not supported will support later
#define VFS_GLOBAL_EXT4_ID		2 //not supported will support later
	
#endif
