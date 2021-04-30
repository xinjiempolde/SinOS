#ifndef ATA_H
#define ATA_H

#include <cpu/type.h>

#define ATA_PRI_BASE_PORT 0x1f0
#define ATA_SEC_BASE_PORT 0x170

typedef struct AtaPorts AtaPorts;
typedef struct AtaBus AtaBus;
typedef struct disk_array disk_array;

struct AtaPorts {
    uint16_t data_port;
    uint16_t error_port;
    uint16_t feautre_port;
    uint16_t sector_count_port;
    uint16_t LBAlo_port;
    uint16_t LBAmid_port;
    uint16_t LBAhi_port;
    uint16_t drive_port;
    uint16_t status_port;
    uint16_t command_port;
    uint16_t control_port;
};

struct AtaBus {
    bool is_master;
    uint16_t io_base_port;
    AtaPorts ata_ports;
};

struct disk_array {
    bool is_exists[4];
    AtaBus bus_array[4];
};

void init_all_disk(disk_array* disks);
void init_ata_bus(AtaBus* ataBus, bool is_master, uint16_t base_port);
bool ata_identify(AtaBus* bus);
void ata_write(AtaBus* bus, uint32_t LBA, uint8_t* data, int nbytes);
void ata_read(AtaBus* bus, uint32_t LBA, uint8_t* buf, int nbytes);
void ata_flush(AtaBus* bus);
#endif