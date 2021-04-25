#include <kernel/pci.h>
#include <drivers/ports.h>
#include <gui/vga.h>

uint32_t r_pci_32(uint8_t bus, uint8_t device, uint8_t func, uint8_t pcireg) {
    port_dword_out(PCI_CONFIG_ADDRESS, PCI_ENABLE_BIT | (bus << 16) | (device << 11) | (func << 8) | (pcireg << 2));
    uint32_t ret = port_dword_in(PCI_CONFIG_DATA);

    return ret;
}

int init_pcilist(void) {
    uint8_t bus, device, func;
    uint32_t data;
    for (bus = 0; bus != 0xff; bus++) {
        for (device = 0; device < 32; device++) {
            for (func = 0; func < 8; func++) {
                data = r_pci_32(bus, device, func, 0);

                if (data != 0xffffffff) {
                    //printf("bus %d, device %d, func %d, data:%x", bus, device, func, data);
                    data = r_pci_32(bus, device, func, 2);
                    printf("data: %x", data);
                }
            }
        }
    }
}