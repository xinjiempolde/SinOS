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
super_block sb;
/**
 * 初始化文件系统
 */
void init_fs() {

    int i;

    /* 写入超级块 */
    init_all_disk(&disks);
    sb.magic = 0x66;
    sb.sec_cnt = 0x400000; // 80M
    sb.inode_cnt = 0x4000; // 4K, 最多4096个文件

    sb.block_bitmap_lba = 1; // 空闲块扇区位图的起始lba
    sb.block_bitmap_sects = 20; // 空闲块扇区位图所占的扇区数量

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects; // inode位图的起始lba,21
    sb.inode_bitmap_sects = 1; // inode位图所占扇区数量

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects; // inode数组的起始lba, 22
    sb.inode_table_sects = 512; // inode数组所占的扇区数量

    sb.dir_entry_lba = sb.inode_table_lba + sb.inode_table_sects; // 根目录区的起始lba, 534
    sb.dir_entry_secs = (sb.inode_cnt * sizeof(dir_entry)) / 512; // 根目录区所占有的扇区数
    if (sb.inode_cnt * sizeof(dir_entry) % 512 != 0) {            // 向上取整
        sb.dir_entry_secs += 1;
    }

    sb.data_start_lba = sb.dir_entry_lba + sb.dir_entry_secs; // 数据区的起始地址

    ata_write(&disks.bus_array[2], 0, (char*)&sb, 512);

    /* 写入空闲块位图 */   
    char zero[512];
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

    /* 写入根目录 */
    for (i = sb.dir_entry_lba; i < sb.dir_entry_lba + sb.dir_entry_secs; i++) {
        ata_write(&disks.bus_array[2], i, zero, 512);
    }
}

/* 显示根目录区存在的所有文件 */
void display_all_files(void) {
    int i, j;
    uint8_t mask;
    uint8_t sector[512];
    dir_entry* directory = mem_alloc_4k((MemMan*)MEM_MAN_ADDR, sb.inode_cnt * sizeof(dir_entry));
    ata_read(&disks.bus_array[2], sb.dir_entry_lba, directory, 512);

    ata_read(&disks.bus_array[2], sb.inode_bitmap_lba, sector, 512);
    for (i = 0; i < 512; i++) {
        mask = 0x80;
        for (j = 0; j < 8; j++) {
            if (sector[i] & mask) {
                printf("filename: %s", directory[8 * i + j].filename);
            }
            mask = mask >> 1;
        }
    } 
}

/* 将dir_entry写入磁盘，offset代表是第几个dir_entry */
void write_dir_entry(dir_entry* dir, int offset) {
    uint8_t sector[512]; // 存储读取的一个扇区
    int sector_no = offset * sizeof(dir_entry) / 512;
    int inode_bit_offset = offset / 8;
    ata_read(&disks.bus_array[2], sb.dir_entry_lba + sector_no, sector, 512); // 只读入属于的那一个扇区

    ((dir_entry*)sector)[offset] = *dir; // 使用[]后不必再使用*了，切记

    ata_write(&disks.bus_array[2], sb.dir_entry_lba + sector_no, (uint8_t*)sector, 512); // 写入根目录区

}

/* 分配一个空闲的inode */
int alloc_inode() {
    uint8_t sector[512];
    int i, j;
    uint8_t mask;
    ata_read(&disks.bus_array[2], sb.inode_bitmap_lba, sector, 512);
    for (i = 0; i < 512; i++) {
        mask = 0x80;
        for (j = 0; j < 8; j++) {
            if ((sector[i] & mask) == 0) {
                sector[i] |= mask;
                ata_write(&disks.bus_array[2], sb.inode_bitmap_lba, sector, 512);
                return 8 * i + j; // 返回第几个inode
            }
            mask = mask >> 1;
        }
    }
    
    return -1; // 分配失败
}

// 先试试一个小于512字节的文件，简单一点
void create_file(char* filename, uint8_t* content, int nbytes) {
    dir_entry file;
    int block_no;
    uint8_t sector[512];
    memory_copy(filename, file.filename, strlen(filename));
    file.f_type = FT_FILE;
    file.i_no = alloc_inode();
    file.file_byte_size = nbytes;


    /* 将文件的信息写到根目录区 */
    write_dir_entry(&file, file.i_no);

    /* 将文件的内容写到数据块区域 */
    ata_read(&disks.bus_array[2], sb.inode_table_lba, sector, 512);
    block_no = alloc_content_block();
    ((inode*)sector)[file.i_no].i_sectors[0] = block_no;
    memory_copy(content, sector, nbytes);
    ata_write(&disks.bus_array[2], sb.data_start_lba + block_no, content, nbytes);
    
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