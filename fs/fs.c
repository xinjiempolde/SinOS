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

    sb->block_bitmap_lba = 1; // 空闲块扇区位图的起始lba
    sb->block_bitmap_sects = 20; // 空闲块扇区位图所占的扇区数量

    sb->inode_bitmap_lba = sb->block_bitmap_lba + sb->block_bitmap_sects; // inode位图的起始lba,21
    sb->inode_bitmap_sects = 1; // inode位图所占扇区数量

    sb->inode_table_lba = sb->inode_bitmap_lba + sb->inode_bitmap_sects; // inode数组的起始lba, 22
    sb->inode_table_sects = 512; // inode数组所占的扇区数量
    sb->inodenum_per_sec =  512 / sizeof(inode);

    sb->dir_entry_lba = sb->inode_table_lba + sb->inode_table_sects; // 根目录区的起始lba, 534
    sb->dir_entry_secs = (sb->inode_cnt * sizeof(dir_entry)) / 512; // 根目录区所占有的扇区数, 256个扇区

    sb->data_start_lba = sb->dir_entry_lba + sb->dir_entry_secs; // 数据区的起始地址
}

/**
 * 初始化文件系统
 */
void init_fs() {

    unsigned int i;

    init_all_disk(&disks);
    ata_read(&disks.bus_array[2], 0, (uint8_t*)&sb, 512);
    if (sb.magic == 0x66) {
        return; // 已经有文件系统了，不需要初始化
    }
    
    init_sb(&sb);

    ata_write(&disks.bus_array[2], 0, (uint8_t*)&sb, 512);

    /* 写入空闲块位图 */   
    uint8_t zero[512];
    memset(zero, 512);
    for (i = sb.block_bitmap_lba; i < sb.block_bitmap_lba + sb.block_bitmap_sects; i++) { // 一个扇区的空闲块位图可以控制512K的磁盘大小,暂且先试试10M
        ata_write(&disks.bus_array[2], i, zero, 512);
    } 

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
    int i;
    dir_entry* directory = (dir_entry*)mem_alloc_4k((MemMan*)MEM_MAN_ADDR, sb.inode_cnt * sizeof(dir_entry));
    inode cur_inode; // 当前目录对应的inode
    open_inode(cur_dir->i_no, &cur_inode);
    for (i = 0; i < 14; i++) {
        if (cur_inode.i_sectors[i] == 0) {
            break;
        }
        open_dir(cur_inode.i_sectors[i], directory);
        strcp(directory->filename, (char*)&(argv[i].filename), strlen(directory->filename));
        argv[i].f_type = directory->f_type;
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
    uint8_t block_sector[512];
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
        ata_write(&disks.bus_array[2], sb.data_start_lba + block_no, (uint8_t*)block_sector ,512);
    }

    // 不足512字节的扇区
    block_no = alloc_content_block();
    cur_inode.i_sectors[nbytes / 512] = block_no;
    memset(block_sector, 512);
    memory_copy(content + (nbytes / 512) * 512, block_sector, nbytes % 512);
    ata_write(&disks.bus_array[2], sb.data_start_lba + block_no, block_sector, 512);

    cur_inode.i_no = file.i_no;
    cur_inode.i_size = nbytes; // 创建文件的文件大小
    save_inode(cur_inode.i_no, &cur_inode); // 保存该文件的inode

    /* 保存父亲目录的信息 */
    parent_inode.i_sectors[parent_inode.i_size] = file.i_no;
    parent_inode.i_size += 1; // 父亲目录下的文件数目+1
    save_inode(parent_inode_no, &parent_inode); // 保存目录的inode
    
}

/* 写得不完全，需要修改 */
int read_file(int parent_inode_no, char* filename, uint8_t* content, int nbytes) {
    dir_entry parent_dir;
    inode cur_inode;
    int inode_id;
    unsigned int real_bytes; // 实际应该读取的字节数
    unsigned int i;

    open_dir(parent_inode_no, &parent_dir);
    inode_id = search_dir(filename, &parent_dir, FT_FILE);
    if (inode_id == FILE_BUT_DIR || inode_id == FAIL) { // 找到了目录或者完全没有找到
        return inode_id;
    }
    open_inode(inode_id, &cur_inode);

    real_bytes = nbytes >= cur_inode.i_size ? cur_inode.i_size : nbytes; // 如果读取内容超过实际的大小，只读取实际的字节数


    for (i = 0; i < real_bytes / 512; i++) { // 先读取够512字节的
        ata_read(&disks.bus_array[2], sb.data_start_lba + cur_inode.i_sectors[i], content + 512 * i, 512);
    }
    ata_read(&disks.bus_array[2], sb.data_start_lba + cur_inode.i_sectors[i], content + 512 * i, real_bytes % 512); // 不够512字节的扇区
    // printf("content:%s", content);

    return SUCCESS;
}


// 先测试512个扇区
int alloc_content_block() {
    uint8_t sector[512];
    int i, j;
    uint8_t mask;
    ata_read(&disks.bus_array[2], sb.block_bitmap_lba, sector, 512);
    for (i = 0; i < 512; i++) {
        mask = 0x80;
        for (j = 0; j < 8; j++) {
            if ((sector[i] & mask) == 0) {
                sector[i] |= mask;
                ata_write(&disks.bus_array[2], sb.block_bitmap_lba, sector, 512);
                return 8 * i + j; // 返回第几个block_bitmap
            }
            mask = mask >> 1;
        }
    }
    
    return -1; // 分配失败
}

void free_conten_block(int block_id) {
    uint8_t sector[512];
    int sec_cnt = block_id / 512; // 扇区数量的偏移
    int byte_cnt = (block_id % 512) / 8; // 在字节间的偏移
    int off_byte = block_id % 8; // 在字节内的偏移
    uint8_t mask = (0x80 >> off_byte) ^ 0xff;
    ata_read(&disks.bus_array[2], sb.block_bitmap_lba + sec_cnt, sector, 512);

    sector[byte_cnt] &= mask;
    ata_write(&disks.bus_array[2], sb.block_bitmap_lba + sec_cnt, sector, 512);
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
    struct path_node* next_node = (struct path_node*)mem_allocate(memMan, sizeof(struct path_node));
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