#ifndef __FS_FS_H
#define __FS_FS_H

#include <fs/dir.h>
#include <fs/super_block.h>

#define MAX_FILES_PER_PART 4096	    // 每个分区所支持最大创建的文件数
#define BITS_PER_SECTOR 4096	    // 每扇区的位数
#define SECTOR_SIZE 512		    // 扇区字节大小
#define BLOCK_SIZE SECTOR_SIZE	    // 块字节大小

#define ROOT_INODE_ID 0// 根节点的ID，也是根目录的ID

#ifndef FT_FILE
// 目录的类型
#define FT_UNDIFINE 0x00   // 未定义的类型
#define FT_FILE 0x01       // 普通的文件
#define FT_DIRECOTRY 0x02  // 目录
#define FT_SLINK 0x03      // 软连接
#define FT_HLINK 0x04      // 硬链接
#endif

/* 双向链表，用于路径切换 */
struct path_node {
    int inode_id;
    char dir_name[MAX_FILE_NAME_LEN];

    struct path_node* pre;
    struct path_node* next;
};

void init_sb(super_block* sb);
void init_fs(bool value);
void init_group();
int read_all_files(dir_entry* directory, dir_entry argv[]);

void write_dir_entry(dir_entry* dir, int offset);

void create_file(int parent_inode_no, char* filename, uint8_t* content, int nbytes);
int read_file(char* full_path, uint8_t* content, int nbytes);

int alloc_content_block();
void free_conten_block(int block_id);

void free_link_list(struct path_node* head);
void switch_full_path(char* full_path);
void switch_forward(dir_entry* directory);
void switch_backward();

int create_hard_link(char* name, char* srcfile_path);
#endif
