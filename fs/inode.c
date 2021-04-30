#include <fs/inode.h>
#include <drivers/ata.h>
#include <fs/super_block.h>
#include <libc/mem.h>

#include <gui/vga.h>
extern disk_array disks;
extern super_block sb;
/* 从磁盘读取一个inode到缓冲区里面 */
void open_inode(int inode_no, inode* buf) {
    uint8_t sector[512]; // 临时存储一个扇区的数据
    int sector_offset = (inode_no * sizeof(inode)) / 512; // 计算该inode在inode数组区域的扇区偏移数量
    ata_read(&disks.bus_array[2], sb.inode_table_lba + sector_offset, sector, 512); // 从磁盘读取含有该inode的扇区
    memory_copy((uint8_t*)&((inode*)sector)[inode_no % sb.inodenum_per_sec], (uint8_t*)buf, sizeof(inode));
}

void save_inode(int inode_no, inode* to_save_inode) {
    uint8_t sector[512]; // 临时存储一个扇区的数据
    int sector_offset = (inode_no * sizeof(inode)) / 512; // 计算该inode在inode数组区域的扇区偏移数量
    ata_read(&disks.bus_array[2], sb.inode_table_lba + sector_offset, sector, 512);
    ((inode*)sector)[inode_no % sb.inodenum_per_sec] = *to_save_inode;
    ata_write(&disks.bus_array[2], sb.inode_table_lba + sector_offset, sector, 512); // 写回磁盘
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

void free_inode(int inode_no) {
    uint8_t sector[512];
    uint8_t mask = 0x80 >> (inode_no % 8); // 在一个字节内的偏移
    int off_byte = inode_no / 8; // 字节数的偏移
    ata_read(&disks.bus_array[2], sb.inode_bitmap_lba, sector, 512);
    if ((sector[off_byte] & mask) == 0) {
        return; // 该inode没有分配，无需free
    }

    sector[off_byte] &= mask ^ 0xff; // 将该位bitmap清0
    ata_write(&disks.bus_array[2], sb.inode_bitmap_lba, sector, 512); // 保存inode_bitmap到磁盘

    /* 将该节点信息清零 */
    inode zero;
    memset((uint8_t*)&zero, sizeof(inode));
    save_inode(inode_no, &zero);

}