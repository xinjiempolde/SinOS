#include <fs/dir.h>
#include <drivers/ata.h>
#include <fs/super_block.h>
#include <fs/inode.h>
#include <fs/fs.h>
#include <libc/mem.h>
#include <gui/vga.h>
#include <mm/memory.h>
#include <libc/string.h>
extern dir_entry cur_dir; // 当前所在的目录的目录项
extern disk_array disks;
extern super_block sb;
static MemMan* memMan = (MemMan*)MEM_MAN_ADDR;

void save_dir(dir_entry* dir) {
    uint8_t sector[512];

    /* 将目录写入到对应的磁盘中 */
    int num_per_sec = 512 / sizeof(dir_entry);  // 每个扇区有多少个目录项,值为16
    int offset = dir->i_no / num_per_sec;       // 相对于目录起始扇区的偏移数
    int sec_id = sb.dir_entry_lba + offset;     // 对应的扇区lba的ID
    
    ata_read(&disks.bus_array[2], sec_id, sector, 512);
    ((dir_entry*)sector)[dir->i_no % num_per_sec] = *dir;
    ata_write(&disks.bus_array[2], sec_id, sector, 512);
}

void open_dir(int inode_no, dir_entry* buf) {
    uint8_t sector[512];
    int dir_per_sec = 512 / sizeof(dir_entry);  // 一个扇区有多少个目录项(16个)
    int offset = inode_no / dir_per_sec;
    ata_read(&disks.bus_array[2], sb.dir_entry_lba + offset, sector, 512);
    memory_copy((uint8_t*)&(((dir_entry*)sector)[inode_no % dir_per_sec]), (uint8_t*)buf, sizeof(dir_entry));
}

/* 创建一个目录 */
int create_dir(char* dir_name, int parent_no) {
    dir_entry diretory;
    inode parent, cur_inode;
    int i;
    int one_indirect_sec[128] = {0};  // 缓存一级间接寻址扇区，能存储128个int类型的inode编号
    
    dir_entry* tmp = search_dir_by_id(dir_name, parent_no);
    if (tmp != NULL) { // 该目录已存在
        printf("该文件或目录已经存在");
        return FAIL;
    }
    //mem_free_4k(memMan, (uint32_t)tmp, sizeof(dir_entry));
    
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
            if (parent.i_sectors[ONE_INDIRECT_ID] == 0) { // 一级间接寻址数组还没有分配磁盘块
                parent.i_sectors[ONE_INDIRECT_ID] = alloc_content_block();
            } else {    // 如果已经分配了，读取到数组中
                ata_read(&disks.bus_array[2], parent.i_sectors[ONE_INDIRECT_ID], (uint8_t*)one_indirect_sec, 512);
            }
            for (i = 0; i < 128; i++) {
                if (one_indirect_sec[i] == 0) {
                    one_indirect_sec[i] = diretory.i_no;
                    break;
                }

            }
            ata_write(&disks.bus_array[2], parent.i_sectors[ONE_INDIRECT_ID], (uint8_t*)one_indirect_sec, 512);
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
 * 在目录下搜索指定名称的目录项，如果找到，返回目录项dir_entry
 * 若没有找到，返回NULL
 */
dir_entry* search_dir(char* dir_name, dir_entry* current_directory) {
    inode parent_inode;
    dir_entry* sub_dir = (dir_entry*)mem_alloc_4k(memMan, sizeof(dir_entry));
    int i;
    open_inode(current_directory->i_no, &parent_inode);
    for (i = 0; i < DIRECT_BLOCK_NUM; i++) {    // 现从直接索引数组中找inode
        if (parent_inode.i_sectors[i] == 0) {   // 到此处表明已经不寻找inode了，该目录下没有该(目录)文件
            return NULL;
        }
        open_dir(parent_inode.i_sectors[i], sub_dir);
        if (strcmp(dir_name, sub_dir->filename) == 0) {
            return sub_dir; // 只要名字一样，不管类型，直接返回dir_entry;
        }
    }

    /* 在直接索引数组中没有找到，再从一级间接寻址数组中找 */
    int one_indrect_array[128] = {0};
    ata_read(&disks.bus_array[2], parent_inode.i_sectors[ONE_INDIRECT_ID], (uint8_t*)one_indrect_array, 512);
    for (i = 0; i < 128; i++) {
        if (one_indrect_array[i] == 0) {
            return NULL;        // 在一级间接寻址中找不到
        }
        open_dir(one_indrect_array[i], sub_dir);
        if (strcmp(dir_name, sub_dir->filename) == 0) {
            return sub_dir;
        }
    }

    /* 在直接索引和一级间接索引均未找到 */
    return NULL;
    
}

dir_entry* search_dir_by_id(char* dir_name, int parent_inode_id) {
    dir_entry parent;
    open_dir(parent_inode_id, &parent);

    return search_dir(dir_name, &parent);
}

/**
 * 功能:解析目录项的全路径，如果找到返回对应的目录项，没找到返回NULL
 */
dir_entry* parse_full_path(char* full_path) {
    if (strlen(full_path) <= 0) return NULL;

    int i;
    char* argv[10] = {NULL};
    int current_i;

    str_split(full_path, (char**)argv, '/');

    int listLen = str_list_len(argv, 10);
    dir_entry* tmp_dir;

    if (full_path[0] == '/') { // 从根目录开始解析
        current_i = ROOT_INODE_ID;
    } else { // 从当前目录开始解析
        current_i = cur_dir.i_no;
    }

    // 前面的参数均为目录，最后一个参数可能为目录，可能为文件
    for (i = 0; i < listLen - 1; i++) {
        tmp_dir = search_dir_by_id(argv[i], current_i);
        if (tmp_dir == NULL || tmp_dir->f_type != FT_DIRECOTRY) {
            return NULL;
        }
        current_i = tmp_dir->i_no;
    }
    tmp_dir = search_dir_by_id(argv[listLen-1], current_i);
    if (tmp_dir == NULL) {
        return NULL;
    }
    return tmp_dir;
}

/**
 * 删除目录下的一个目录项，parent_no为父目录的inode_id
 */
int rm_dir_by_name(char* rm_dir_name, int parent_no, int type) {
    dir_entry* rm_dir = search_dir_by_id(rm_dir_name, parent_no);
    int rm_inode_id;
    if (rm_dir == NULL) {
        return FAIL;
    } else if (rm_dir->f_type != type){
        return FILE_BUT_DIR;
    }
    rm_inode_id = rm_dir->i_no;
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

    int offset = id / sb.dirnum_per_sec; // 指定目录项的扇区偏移
    int sec_id = sb.dir_entry_lba + offset;

    /* 将目录写入到对应的磁盘中 */
    ata_read(&disks.bus_array[2], sec_id, sector, 512);
    ((dir_entry*)sector)[id % sb.dirnum_per_sec] = empty_dir;
    ata_write(&disks.bus_array[2], sec_id, sector, 512);
}