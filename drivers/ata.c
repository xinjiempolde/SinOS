#include <drivers/ata.h>
#include <drivers/ports.h>
#include <gui/vga.h>
#include <libc/string.h>
#include <libc/function.h>
disk_array disks;

void init_all_disk(disk_array* disk) {
    bool is_exist; // 该总线上是否存在ATA设备
    int i;

    init_ata_bus(&disk->bus_array[0], TRUE, ATA_PRI_BASE_PORT);
        
    init_ata_bus(&disk->bus_array[1], FALSE, ATA_PRI_BASE_PORT);

    init_ata_bus(&disk->bus_array[2], TRUE, ATA_SEC_BASE_PORT);

    init_ata_bus(&disk->bus_array[3], FALSE, ATA_SEC_BASE_PORT);
    for (i = 0; i < 4; i++) {
        disk->is_exists[i] = ata_identify(&disk->bus_array[i]);
        // if (disk->is_exists[i]) {
        //     printf("value: %d", i);
        // }
    }
}

void init_ata_bus(AtaBus* ataBus, bool is_master, uint16_t base_port) {
    AtaPorts* ataPorts = &(ataBus->ata_ports);
    ataBus->io_base_port = base_port;
    ataBus->is_master = is_master;
    ataPorts->data_port = base_port + 0x0;
    ataPorts->error_port = base_port + 0x1;
    ataPorts->feautre_port = base_port + 0x1;
    ataPorts->sector_count_port = base_port + 0x2;
    ataPorts->LBAlo_port = base_port + 0x3;
    ataPorts->LBAmid_port = base_port + 0x4;
    ataPorts->LBAhi_port = base_port + 0x5;
    ataPorts->drive_port = base_port + 0x6;
    ataPorts->status_port = base_port + 0x7;
    ataPorts->command_port = base_port + 0x7;
    ataPorts->control_port = base_port + 0x206;
}

/**
 * idenfify command, see: https://wiki.osdev.org/ATA_PIO_Mode
 */
bool ata_identify(AtaBus* bus) {
    uint16_t data, status;
    int i;
    AtaPorts* ports = &(bus->ata_ports);

    /* detect floating bus */
    status = port_byte_in(ports->status_port);
    if (status == 0xff) {
        // printf("%x %x floating bus", bus->io_base_port, bus->is_master);
        return FALSE;
    }

    /* Detecting Controller IO Ports */
    port_byte_out(ports->LBAlo_port, 0x11);
    port_byte_out(ports->LBAhi_port, 0x22);
    if ((port_byte_in(ports->LBAlo_port) != 0x11)
        && (port_byte_in(ports->LBAhi_port) != 0x22) ) {
            // printf("%x %x no hard disk", bus->io_base_port, bus->is_master);
            return FALSE;
    }

    /* IDENTIFY COMMAND */
    port_byte_out(ports->drive_port, bus->is_master?0xa0:0xb0);
    port_byte_out(ports->sector_count_port, 0);
    port_byte_out(ports->LBAlo_port, 0);
    port_byte_out(ports->LBAmid_port, 0);
    port_byte_out(ports->LBAhi_port, 0);
    port_byte_out(ports->command_port, 0xec);
    status = port_byte_in(ports->status_port);
    /* if status is 0, the drive does not exist */
    if (status == 0) {
        // printf("%x %x no hard disk", bus->io_base_port, bus->is_master);
        return FALSE;
    }
    
    /* polling util bit 7 BSY(0x80) clears or util bit 0 ERR(0x01) sets */
    while ((status & 0x80) == 0x80
            && (status & 0x01) == 0x00) {
        status = port_byte_in(ports->status_port);
    }

    /* bit 0 ERR sets indicates that error occured  */
    if ((status & 0x01) == 0x01) {
        // printf("%x %x ata error", bus->io_base_port, bus->is_master);
        return FALSE;
    }

    char s[512];
    /* Read 256 16-bit values from data port */
    // for (i = 0; i < 256; i++) {
    //     data = port_word_in(ports->data_port);
    //     if (((data & 0xff) != 0) && ((data & 0xff00) != 0) ) {
    //         append(s, (char)((data >> 8) & 0xff));
    //         append(s, (char)(data & 0xff));
    //     }
    //     if (i >= 40) break;
    // }
    // printf(s);
    return TRUE;
}

void ata_write(AtaBus* bus, uint32_t LBA, uint8_t* data, int nbytes) {
    int i;
    uint16_t wdata;
    AtaPorts* ports = &(bus->ata_ports);
    port_byte_out(ports->drive_port, (bus->is_master?0xe0:0xf0) | ((LBA >> 24) & 0x0f));
    port_byte_out(ports->error_port, 0x0);
    port_byte_out(ports->sector_count_port, 1); // just 1 sector
    port_byte_out(ports->LBAlo_port, (LBA & 0xff)); // low 8 bit
    port_byte_out(ports->LBAmid_port, (LBA >> 8) & 0xff); // middle 8 bit
    port_byte_out(ports->LBAhi_port, (LBA >> 16) & 0xff); // high 8 bit
    port_byte_out(ports->command_port, 0x30); // write sectors command

    uint32_t status = port_word_in(ports->status_port);
    /* poll status until ready which has the same function with IRQ */
    while ((status & 0x80) == 0x80
        && (status & 0x01) == 0x00) {
        status = port_word_in(ports->status_port);
    }
    
    for (i = 0; i < nbytes; i+=2) {
        wdata = data[i];
        wdata |= data[i+1] << 8;
        port_word_out(ports->data_port, wdata);
    }

    for (i = nbytes + (nbytes % 2); i < 512; i+=2) {
        port_word_out(ports->data_port, 0x0000);
    }
    ata_flush(bus);
}

void ata_read(AtaBus* bus, uint32_t LBA, char* buf, int nbytes) {
    uint16_t status, data = 0;
    int i;
    AtaPorts* ports = &(bus->ata_ports);
    port_byte_out(ports->drive_port, (bus->is_master?0xe0:0xf0) | ((LBA >> 24) & 0x0f));
    port_byte_out(ports->error_port, 0x0);
    port_byte_out(ports->sector_count_port, 1); // just 1 sector
    port_byte_out(ports->LBAlo_port, (LBA & 0xff)); // low 8 bit
    port_byte_out(ports->LBAmid_port, (LBA >> 8) & 0xff); // middle 8 bit
    port_byte_out(ports->LBAhi_port, (LBA >> 16) & 0xff); // high 8 bit
    port_byte_out(ports->command_port, 0x20); // read sectors command

    status = port_byte_in(ports->status_port);
    /* poll status until ready which has the same function with IRQ */
    while ((status & 0x80) == 0x80
        && (status & 0x01) == 0x00) {
        status = port_byte_in(ports->status_port);
    }
    
    if (status & 0x01) {
        // printf("ata error");
        return;
    }

    for (i = 0; i < nbytes; i+=2) {
        data = port_word_in(ports->data_port);
        buf[i] = (char)(data & 0xff);
        buf[i+1] = (char) ((data >> 8) & 0xff);
    }

    for (i = nbytes + (nbytes % 2); i < 512; i+=2) {
        port_word_in(ports->data_port);
    }

    
}

void ata_flush(AtaBus* bus) {
    AtaPorts* ports = &(bus->ata_ports);
    uint16_t status;
    port_byte_out(ports->drive_port, bus->is_master?0xe0:0xf0);
    port_byte_out(ports->command_port, 0xe7);

    status = port_byte_in(ports->status_port);
    while ((status & 0x80) == 0x80
        && (status & 0x01) == 0x00) {
        status = port_byte_in(ports->status_port);
    }
    return;
}