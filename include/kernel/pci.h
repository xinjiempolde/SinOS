#ifndef PCI_H
#define PCI_H

#define PCI_ENABLE_BIT 0x80000000
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#include <cpu/type.h>

uint32_t r_pci_32(uint8_t bus, uint8_t device, uint8_t func, uint8_t pcireg);
int init_pcilist(void);
#endif