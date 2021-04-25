#ifndef __FS_DIR_H
#define __FS_DIR_H

#include <cpu/type.h>
#define MAX_FILE_NAME_LEN  15	 // 最大文件名长度

/* 目录结构 */
struct dir {
   struct inode* inode;   
   uint32_t dir_pos;	  // 记录在目录内的偏移
   uint8_t dir_buf[512];  // 目录的数据缓存
};

/* 目录项结构 */
/* 凑了凑变成32字节，不要问我为什么，不对齐16字节看16进制太难受了 */
typedef struct {
   char filename[MAX_FILE_NAME_LEN];  // 普通文件或目录名称
   uint32_t i_no;		      // 普通文件或目录对应的inode编号
   uint32_t create_time;   // 创建的时间
   uint32_t modify_time;   // 修改时间
   uint32_t file_byte_size;// 文件大小(字节)
   uint8_t f_type;	      // 文件类型
} __attribute__((packed)) dir_entry;

#endif
