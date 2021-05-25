#include <fs/dir.h>
#include <drivers/ata.h>
#include <fs/super_block.h>
#include <fs/inode.h>
#include <fs/fs.h>
#include <libc/mem.h>
#include <gui/vga.h>
#include <libc/string.h>
extern disk_array disks;
extern super_block sb;

void save_dir(dir_entry* dir) {
    uint8_t sector[512];

    /* 将目录写入到对应的磁盘中 */
    ata_read(&disks.bus_array[2], sb.dir_entry_lba, sector, 512);
    ((dir_entry*)sector)[dir->i_no] = *dir;
    ata_write(&disks.bus_array[2], sb.dir_entry_lba, sector, 512);
}

void open_dir(int inode_no, dir_entry* buf) {
    uint8_t sector[512];
    ata_read(&disks.bus_array[2], sb.dir_entry_lba, sector, 512);
    memory_copy((uint8_t*)&(((dir_entry*)sector)[inode_no]), (uint8_t*)buf, sizeof(dir_entry));
}

/* 创建一个目录 */
int create_dir(char* dir_name, int parent_no) {
    dir_entry diretory;
    inode parent, cur_inode;
    int i;
    int* one_indirect_sec[128];  // 缓存一级间接寻址扇区，能存储128个int类型的inode编号
    
    if (search_dir_by_id(dir_name, parent_no, FT_DIRECOTRY) != FAIL) { // 该目录已存在
        return FAIL;
    }
    
    memset((uint8_t*)&diretory, sizeof(diretory)); // 清0， 防止栈中残留的脏数据

    /* 准备待创建的目录结构 */
    diretory.create_time = 0;
    diretory.modify_time = 0;
    diretory.f_type = FT_DIRECOTRY;
    diretory.i_no = alloc_inode();
    diretory.file_byte_size = 0;
    memory_copy((uint8_t*)dir_name, (uint8_t*)diretory.filename, strlen(dir_name));

    /* 将目录写入到对应的磁盘中 */
    save_dir(&diretory);

    if (parent_no >= 0) {
        open_inode(parent_no, &parent);
        for (i = 0; i < DIRECT_BLOCK_NUM; i++) {
            if (parent.i_sectors[i] == 0) { // 找到该目录结构下还没使用的inode编号
                parent.i_sectors[i] = diretory.i_no;
                break;
            }
        }

        /* 如果直接寻址数组已满，寻找一级间接寻址数组 */
        if (i == DIRECT_BLOCK_NUM) {
            if (parent.i_sectors[ONE_INDIRECT_ID] == 0) { // 一级间接寻址数组还没有分配磁盘快
                parent.i_sectors[ONE_INDIRECT_ID] = alloc_content_block();
            }
            ata_read(&disks.bus_array[2], parent.i_sectors[ONE_INDIRECT_ID], (uint8_t*)one_indirect_sec, 512);
        }
        parent.i_size += 1; // 父亲目录下的文件数量+1
        save_inode(parent_no, &parent);
    }

    /* 将当前inode保存到磁盘中 */
    memset((uint8_t*)&cur_inode, sizeof(cur_inode));
    cur_inode.i_no = diretory.i_no;
    cur_inode.i_size = 0;
    save_inode(cur_inode.i_no, &cur_inode);

    return SUCCESS;
}

/**
 * 在目录下搜索指定名称的目录项，如果找到，返回inode编号，否则返回-1
 * type有两种值，要么为文件类型，要么为目录类型
 */
int search_dir(char* dir_name, dir_entry* current_directory, int type) {
    inode parent_inode;
    dir_entry sub_dir;
    int i;
    open_inode(current_directory->i_no, &parent_inode);
    for (i = 0; i < 14; i++) {
        if (parent_inode.i_sectors[i] == 0) {
            break;
        }
        open_dir(parent_inode.i_sectors[i], &sub_dir);
        if (strcmp(dir_name, sub_dir.filename) == 0) {
            if (sub_dir.f_type == type) {
                return parent_inode.i_sectors[i];
            } else {
                return type == FT_FILE? FILE_BUT_DIR : DIR_BUT_FILE;
            }
        }
    }
    return FAIL;
    
}

int search_dir_by_id(char* dir_name, int parent_inode_id, int type) {
    dir_entry parent;
    open_dir(parent_inode_id, &parent);

    return search_dir(dir_name, &parent, type);
}

/**
 * 删除目录下的一个目录项，parent_no为父目录的inode_id
 */
int rm_dir_by_name(char* rm_dir_name, int parent_no, int type) {
    int rm_inode_id =  search_dir_by_id(rm_dir_name, parent_no, type);
    if (rm_inode_id == FAIL || rm_inode_id == FILE_BUT_DIR || rm_inode_id == DIR_BUT_FILE) {
        return rm_inode_id;
    }
    return rm_dir_by_id(rm_inode_id, parent_no, type);
}

int rm_dir_by_id(int rm_id, int parent_no, int type) {
    unsigned int i, j;
    inode parent_inode, cur_inode;

    /* 如果删除的是文件，需要处理block_bitmap */
    if (type == FT_FILE) {
        open_inode(rm_id, &cur_inode);
        for (i = 0; i < cur_inode.i_size; i++) {
            free_conten_block(cur_inode.i_sectors[i]);
        }    
    }

    zero_dir(rm_id); // 清空目录
    free_inode(rm_id); // 清空inode
    open_inode(parent_no, &parent_inode);

    
    /* 将本目录项的inode从父亲inode中移除 */
    for (i = 0; i < parent_inode.i_size; i++) {
        if (parent_inode.i_sectors[i] == (unsigned int)rm_id) {
            // 后面的元素往前移动
            for (j = i; j < parent_inode.i_size - 1; j++) {
                parent_inode.i_sectors[j] = parent_inode.i_sectors[j+1];
            }
            parent_inode.i_sectors[j] = 0;
            break;
        }
    }
    parent_inode.i_size--;
    save_inode(parent_no, &parent_inode);

    return 0;

}

/* 将指定id的目录项清0 */
void zero_dir(int id) {
    uint8_t sector[512];

    dir_entry empty_dir;
    memset((uint8_t*)&empty_dir, sizeof(dir_entry));

    /* 将目录写入到对应的磁盘中 */
    ata_read(&disks.bus_array[2], sb.dir_entry_lba, sector, 512);
    ((dir_entry*)sector)[id] = empty_dir;
    ata_write(&disks.bus_array[2], sb.dir_entry_lba, sector, 512);
}