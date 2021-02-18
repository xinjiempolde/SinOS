#include <drivers/ata.h>
#include <drivers/ports.h>

void identify(void) {
    port_byte_out(ATA_PRI_DRIVE_PORT, 0xa0);
    
}