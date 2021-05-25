#ifndef __FS_INODE_H
#define __FS_INODE_H
#include <cpu/type.h>

#define DIRECT_BLOCK_NUM 13      // 直接寻址块的数量
#define ONE_INDIRECT_ID 13       // 一级间接寻址的下标

typedef struct inode inode;
/* inode结构 */
/* 凑成64个字节，方便~ */
struct inode {
   uint32_t i_no;    // inode编号

   /* 当此inode是文件时,i_size是指文件大小,
   若此inode是目录,i_size是指该目录下所有目录项大小之和*/
   uint32_t i_size;

   /**
    * 如果是目录文件的inode，sectors存放该目录下所有子文件的inode编号
    * 如果是普通文件，存放为该文件分配的所有空闲块的编号
    * 0~12前13块为直接寻址，第13块为一级间接寻址
    */
   uint32_t i_sectors[14];
} __attribute__((packed));

void open_inode(int inode_no, inode* buf);
void save_inode(int inode_no, inode* to_save_inode);
int alloc_inode();
void free_inode(int inode_no);
#endif