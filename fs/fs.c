#include <fs/fs.h>
#include <fs/super_block.h>
#include <fs/inode.h>
#include <fs/dir.h>
#include <drivers/ata.h>
#include <gui/vga.h>
#include <libc/mem.h>
#include <mm/memory.h>
#include <libc/string.h>
extern disk_array disks;
static MemMan* memMan = (MemMan*)MEM_MAN_ADDR;
super_block sb;
dir_entry cur_dir; // 当前所在的目录的目录项
struct path_node path_linked_list; // 路径头指针
struct path_node* path_tail; // 路径尾指针
/* 初始化超级块，不写入磁盘 */
void init_sb(super_block* sb) {

    /* 超级块在内存的初始化 */
    init_all_disk(&disks);
    sb->magic = 0x66;
    sb->sec_cnt = 0x400000; // 80M
    sb->inode_cnt = 0x4000; // 4K, 最多4096个文件


    sb->inode_bitmap_lba = 2; // inode位图的起始lba,21
    sb->inode_bitmap_sects = 1; // inode位图所占扇区数量

    sb->inode_table_lba = sb->inode_bitmap_lba + sb->inode_bitmap_sects; // inode数组的起始lba, 22
    sb->inode_table_sects = 512; // inode数组所占的扇区数量
    sb->inodenum_per_sec =  512 / sizeof(inode);

    sb->dir_entry_lba = sb->inode_table_lba + sb->inode_table_sects; // 根目录区的起始lba, 534
    sb->dir_entry_secs = (sb->inode_cnt * sizeof(dir_entry)) / 512; // 根目录区所占有的扇区数, 256个扇区
    sb->dirnum_per_sec = 512 / sizeof(dir_entry); // 一个扇区有多少个目录项

    sb->data_start_lba = sb->dir_entry_lba + sb->dir_entry_secs; // 数据区的起始地址

    sb->free[0] = 100;
    int i;
    for (i = 1; i < NGROUP+1; i++) {
        sb->free[i] = sb->data_start_lba + NGROUP - i;
    }
}

/**
 * 功能：初始化成组链接的组，假定超级块中的组已初始化
 */
void init_group() {
    int i;
    int leader_sec_id = sb.free[1]; // 组长块的扇区ID
    int free_start_sec_id = leader_sec_id + 1; // 下一个空闲块的编号
    GroupStack data;
    data.free[0] = 100;

    while (1) {
        for (i = 1; i < NGROUP+1; i++) {
            data.free[i] = free_start_sec_id + NGROUP - i;
        }
        ata_write(&disks.bus_array[2], leader_sec_id, (uint8_t*)&data, 512);
        leader_sec_id = data.free[1];
        free_start_sec_id = leader_sec_id + 1;

        if (leader_sec_id > 4000) {
            break;
        }
    }
}

/**
 * 初始化文件系统
 */
void init_fs(bool value) {

    unsigned int i;

    init_all_disk(&disks);
    ata_read(&disks.bus_array[2], SB_LBA, (uint8_t*)&sb, 512);
    if (sb.magic == 0x66 && !value) {
        return; // 已经有文件系统了，不需要初始化
    }
    
    init_sb(&sb);
    init_group();

    /* 写入超级块 */
    ata_write(&disks.bus_array[2], SB_LBA, (uint8_t*)&sb, 512);


    uint8_t zero[512];
    memset(zero, 512);

    /* 写入inode位图 */
    ata_write(&disks.bus_array[2], sb.inode_bitmap_lba, zero, 512); // 一个inode位图扇区可以代表4096个inode，先设置一个扇区再说

    /* 写入inode数组 */
    for (i = sb.inode_table_lba; i < sb.inode_table_lba + sb.inode_table_sects; i++) {
        ata_write(&disks.bus_array[2], i, zero, 512);
    }

    /* 写入目录区 */
    for (i = sb.dir_entry_lba; i < sb.dir_entry_lba + sb.dir_entry_secs; i++) {
        ata_write(&disks.bus_array[2], i, zero, 512);
    }

    create_dir("/", 0); // 创建根目录
    open_dir(ROOT_INODE_ID, &cur_dir); // 设置当前的目录

    // 初始化路径双向链表
    memory_copy((uint8_t*)cur_dir.filename, (uint8_t*)path_linked_list.dir_name, strlen(cur_dir.filename));
    path_linked_list.inode_id = cur_dir.i_no;
    path_linked_list.pre = NULL;
    path_linked_list.next = NULL;

    path_tail = &path_linked_list;

    
}

/* 显示指定目录区存在的所有文件, 保存到argv字符串数组中，返回文件个数 */
int read_all_files(dir_entry* cur_dir, dir_entry argv[]) {
    int i, j;
    dir_entry* directory = (dir_entry*)mem_alloc_4k((MemMan*)MEM_MAN_ADDR, sb.inode_cnt * sizeof(dir_entry));
    inode cur_inode; // 当前目录对应的inode
    open_inode(cur_dir->i_no, &cur_inode);
    for (i = 0; i < DIRECT_BLOCK_NUM; i++) {
        if (cur_inode.i_sectors[i] == 0) {
            break;
        }
        open_dir(cur_inode.i_sectors[i], directory);
        strcp(directory->filename, (char*)&(argv[i].filename), strlen(directory->filename));
        argv[i].f_type = directory->f_type;
        argv[i].i_no = cur_inode.i_sectors[i];
    }

    if (cur_inode.i_sectors[ONE_INDIRECT_ID] == 0) {    // 文件的数量还不足以使用一级间接索引
        return i;
    }

    int one_indirect_sec[128] = {0};
    ata_read(&disks.bus_array[2], cur_inode.i_sectors[ONE_INDIRECT_ID], (uint8_t*)one_indirect_sec, 512);
    for (j = 0; j < 128; j++) {
        if (one_indirect_sec[j] == 0) {
            break;
        }
        open_dir(one_indirect_sec[j], directory);
        strcp(directory->filename, (char*)&(argv[i].filename), strlen(directory->filename));
        argv[i].f_type = directory->f_type;
        argv[i].i_no = one_indirect_sec[j];
        i++;
    }

    return i;
}

/* 将dir_entry写入磁盘，offset代表是第几个dir_entry */
void write_dir_entry(dir_entry* dir, int offset) {
    uint8_t sector[512]; // 存储读取的一个扇区
    int sector_no = offset * sizeof(dir_entry) / 512; // 该dir所在的扇区
    int dircnt_per_sector = 512 / sizeof(dir_entry); // 每个扇区有多少个目录项
    ata_read(&disks.bus_array[2], sb.dir_entry_lba + sector_no, (uint8_t*)sector, 512); // 只读入属于的那一个扇区

    ((dir_entry*)sector)[offset % dircnt_per_sector] = *dir; // 使用[]后不必再使用*了，切记

    ata_write(&disks.bus_array[2], sb.dir_entry_lba + sector_no, (uint8_t*)sector, 512); // 写入根目录区

}


// 创建一个文件
void create_file(int parent_inode_no, char* filename, uint8_t* content, int nbytes) {
    dir_entry file;
    inode parent_inode, cur_inode;
    uint8_t block_sector[512] = {0};
    int block_no;
    int i;

    /* 准备该文件的相关信息 */
    memset((uint8_t*)&file, sizeof(dir_entry));
    memory_copy((uint8_t*)filename, (uint8_t*)file.filename, strlen(filename));
    file.f_type = FT_FILE;
    file.i_no = alloc_inode();
    file.file_byte_size = nbytes;


    /* 将文件的信息写到根目录区 */
    save_dir(&file);

    open_inode(parent_inode_no, &parent_inode); // 打开当前的目录节点

    memset((uint8_t*)&cur_inode, sizeof(cur_inode));


    /* 分配空闲数据块 */
    // 先写前面够512字节的扇区
    for (i = 0; i < nbytes / 512; i++) {
        block_no = alloc_content_block();
        cur_inode.i_sectors[i] = block_no;

        memory_copy(content + i * 512, block_sector, 512);
        ata_write(&disks.bus_array[2], block_no, (uint8_t*)block_sector ,512);
    }

    // 不足512字节的扇区
    block_no = alloc_content_block();
    cur_inode.i_sectors[nbytes / 512] = block_no;
    memset(block_sector, 512);
    memory_copy(content + (nbytes / 512) * 512, block_sector, nbytes % 512);
    ata_write(&disks.bus_array[2], block_no, block_sector, 512);

    cur_inode.i_no = file.i_no;
    cur_inode.i_size = nbytes; // 创建文件的文件大小
    save_inode(cur_inode.i_no, &cur_inode); // 保存该文件的inode

    /* 保存父亲目录的信息 */
    parent_inode.i_sectors[parent_inode.i_size] = file.i_no;
    parent_inode.i_size += 1; // 父亲目录下的文件数目+1
    save_inode(parent_inode_no, &parent_inode); // 保存目录的inode
    
}

/* 写得不完全，需要修改 */
int read_file(char* full_path, uint8_t* content, int nbytes) {
    inode cur_inode;
    int inode_id;
    unsigned int real_bytes; // 实际应该读取的字节数
    unsigned int i;

    dir_entry* dir = parse_full_path(full_path);
    if (dir == NULL) {
        return FAIL;
    } else if (dir->f_type != FT_FILE) {
        return FILE_BUT_DIR;
    }

    inode_id = dir->i_no;
    open_inode(inode_id, &cur_inode);

    real_bytes = nbytes >= cur_inode.i_size ? cur_inode.i_size : nbytes; // 如果读取内容超过实际的大小，只读取实际的字节数


    for (i = 0; i < real_bytes / 512; i++) { // 先读取够512字节的
        ata_read(&disks.bus_array[2], cur_inode.i_sectors[i], content + 512 * i, 512);
    }
    ata_read(&disks.bus_array[2], cur_inode.i_sectors[i], content + 512 * i, real_bytes % 512); // 不够512字节的扇区
    return SUCCESS;
}


// 需要进行修改
int alloc_content_block() {
    int r;
    if (sb.free[0] == 1) {
        if (sb.free[1] == 0) {
            printf("分配失败，请等待");
        } else {
            r = sb.free[1];
            ata_write(&disks.bus_array[2], r, (uint8_t*)sb.free, sizeof(uint32_t) * (NGROUP+1));
            return r;
        }
    } else {
        r = sb.free[sb.free[0]];
        sb.free[0]--;
        return r;
    }

    ata_write(&disks.bus_array[2], SB_LBA, (uint8_t*)&sb, 512);
}

/* 需要进行修改 */
void free_conten_block(int block_id) {
    if (sb.free[0] == 100) {
        GroupStack data;
        memory_copy((uint8_t*)sb.free, (uint8_t*)&data, sizeof(uint32_t) * (NGROUP+1));
        ata_write(&disks.bus_array[2], block_id, (uint8_t*)&data, 512);
    } else {
        sb.free[0]++;
        sb.free[sb.free[0]] = block_id;
        ata_write(&disks.bus_array[2], SB_LBA, (uint8_t*)&sb, 512);
    }
}

void free_link_list(struct path_node* head) {
    struct path_node* p = head->next;

    if (!p) {
        return; // 只有/节点
    }

    struct path_node* next = p->next;
    while (p) {
        //mem_free_4k(memMan, (uint32_t)p, sizeof(struct path_node));
        p = next;
        next = next->next;
    }
    head->next = NULL;
}

/* 给出全路径，切换到当前路径 */
void switch_full_path(char* full_path) {
    int i, path_len;
    char* argv[10] = {NULL};
    path_len = str_split(full_path, (char**)argv, '/');

    dir_entry* entry;
    if (full_path[0] == '/') { // 从根路径开始
        free_link_list(&path_linked_list);
        cur_dir.i_no = ROOT_INODE_ID;

        for (i = 0; i < path_len; i++) {
            entry = search_dir_by_id(argv[i], cur_dir.i_no);
            if (entry == NULL) {
                printf("file no found");
                return;
            } else {
                printf("name: %s", entry->filename);
                switch_forward(entry);
            }
        }
    } else { // 从当前目录开始
        for (i = 0; i < path_len; i++) {
            switch_forward(search_dir_by_id(argv[i], cur_dir.i_no));
        }
    }
}

/* 向后切换路径 */
void switch_forward(dir_entry* directory) {
    cur_dir = *directory;
    struct path_node* p = &path_linked_list;

    /* 找到末尾 */
    while (p->next) {
        p = p->next;
    }

    /* 创建新的节点 */
    struct path_node* next_node = (struct path_node*)mem_alloc_4k(memMan, sizeof(struct path_node));
    p->next = next_node;
    memory_copy((uint8_t*)&cur_dir.filename, (uint8_t*)&(next_node->dir_name), strlen(cur_dir.filename));
    next_node->inode_id = directory->i_no;
    next_node->pre = p;
    next_node->next = NULL;

    path_tail = next_node;
}

/* 向前切换路径 */
void switch_backward() {
    int inode_id;
    if (path_tail->pre == NULL) inode_id = path_tail->inode_id;
    else {
        path_tail = path_tail->pre;
        path_tail->next->pre = NULL;
        path_tail->next = NULL;
        inode_id = path_tail->inode_id;
    }

    open_dir(inode_id, &cur_dir);
}