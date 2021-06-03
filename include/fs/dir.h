#ifndef __FS_DIR_H
#define __FS_DIR_H

#include <cpu/type.h>
#define MAX_FILE_NAME_LEN  15	 // 最大文件名长度

#ifndef FT_FILE
#define FT_UNDIFINE 0x00   // 未定义的类型
#define FT_FILE 0x01       // 普通的文件
#define FT_DIRECOTRY 0x02  // 目录
#define FT_SLINK 0x03      // 软链接
#define FT_HLINK 0x04      // 硬链接
#endif

#define SUCCESS 0 // 成功找到了对应类型的目录项
#define FILE_BUT_DIR -1 // 需要文件类型却找到了目录
#define DIR_BUT_FILE -2 // 需要目录类型却找到了文件
#define FAIL -3 // 什么也没有找到
#define NO_PERMISSION -4 // 没有权限

#define FILE_FOUND 1
#define FILE_NOFOUND 0


/* 目录项结构 */
/* 凑了凑变成32字节,便于查看二进制debug */
typedef struct {
   char filename[MAX_FILE_NAME_LEN];  // 普通文件或目录名称
   uint32_t i_no;		      // 普通文件或目录对应的inode编号
   uint32_t create_time;   // 创建的时间
   uint32_t user_id;   // 修改时间
   uint32_t file_byte_size;// 文件大小(字节)
   uint8_t f_type;	      // 文件类型
} __attribute__((packed)) dir_entry;


void open_dir(int inode_no, dir_entry* buf);
void save_dir(dir_entry* dir);
void zero_dir(int id);
int create_dir(char* dir_name, int parent_no);
void add_exist_inode(int parent_no, int add_id);
dir_entry* search_dir_by_id(char* dir_name, int parent_inode_id);
dir_entry* search_dir(char* dir_name, dir_entry* current_directory);
dir_entry* parse_full_path(char* full_path);
dir_entry* get_parent_dir(char* full_path);

int rm_dir_by_id(int rm_id, int parent_no, int type);
int rm_dir_by_name(char* rm_dir_name, int parent_no, int type);
void rm_dir_without_free(int rm_id, int parent_no);
#endif
