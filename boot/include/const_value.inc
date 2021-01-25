
;===================================================================================================================================
;const value
;===================================================================================================================================
DeltaDataSecNo    equ 31                     ; boot sector(1 sectors) + FAT1(8 sectors) + FAT2(8 sectors) + root dir(14 sectors) - 2
                                             ; remember 0x0002 is the first sector of data area
SecNumsOfEntry    equ 14                     ; sector numbers of entries is 14, 224*32 bytes = 14 sectors(512 byte/secotr) 
StartSecOfRootDir equ 19                     ; starting from 19-th sector (boot sector(1 byte) + FAT1(9 byte) + FAT2(9 byte)
StartSecOfFAT     equ 1                      ; boot sector(1 sector)
SecNumsOfFATs     equ 18                     ; FAT1(9 sectors) + FAT2(9 sectors)

BaseOfFat         equ 0x0800
OffsetOfFat       equ 0x0000                 ; FAt1 and FAT2: 0x08000~0x0a400

BaseOfRootDir     equ 0x0b00
OffsetOfRootDir   equ 0x0000                 ; Root Directory Entries: 0x0b000~0x0cc00

BaseOfLoader      equ 0x0d00                      
OffsetOfLoader    equ 0x0000                 ; LOADER.BIN: 0x0d000~0x10000, 12KB
LoaderAddress     equ 0xd000

BaseOfKernel      equ 0x1000
OffsetOfKernel    equ 0x0000
KernelAddress     equ 0x10000