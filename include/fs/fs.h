#ifndef __FS_FS_H
#define __FS_FS_H

#define MAX_FILES_PER_PART 4096	    // 每个分区所支持最大创建的文件数
#define BITS_PER_SECTOR 4096	    // 每扇区的位数
#define SECTOR_SIZE 512		    // 扇区字节大小
#define BLOCK_SIZE SECTOR_SIZE	    // 块字节大小

// 文件类型 file type
#define FT_UNDIFINE 0x00   // 未定义的类型
#define FT_FILE 0x01       // 普通的文件
#define FT_DIRECOTRY 0x02  // 目录

#include <fs/dir.h>

void init_fs(void);
void display_all_files(void);
void write_dir_entry(dir_entry* dir, int offset);

void create_file(char* filename, uint8_t* content, int nbytes);

int alloc_inode();
int alloc_content_block();
#endif
