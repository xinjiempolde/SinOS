#ifndef __FS_INODE_H
#define __FS_INODE_H
#include <cpu/type.h>

typedef struct inode inode;
/* inode结构 */
/* 凑成64个字节，方便~ */
struct inode {
   uint32_t i_no;    // inode编号

   /* 当此inode是文件时,i_size是指文件大小,
   若此inode是目录,i_size是指该目录下所有目录项大小之和*/
   uint32_t i_size;

   /* 该文件占有的数据块编号 */
   uint32_t i_sectors[14];
} __attribute__((packed));

void open_inode(int inode_no, inode* buf);
void save_inode(int inode_no, inode* to_save_inode);
int alloc_inode();
void free_inode(int inode_no);
#endif