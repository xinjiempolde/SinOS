; BPB (BIOS Parameter Block)
; ------------------------------------------------------------------------------------------------------------------------------------------------------------
jmp bootsect_start                           ; 3 bytes; short jmp
nop                                          ; nop adds to 3 bytes

BS_OEMName     db 'singheat'                 ; 8 bytes; This is only a name
BPB_BytsPerSec dw 512                        ; 2 bytes; Bytes Per Sectors
BPB_SecPerClus db 1                          ; 1 bytes; Sectors Number Per Cluster
BPB_RsvdSecCnt dw 1                          ; 2 bytes; Number of sectors in reserved area
BPB_NumFATs    db 2                          ; 1 bytes; Number of FATs
BPB_RootEntCnt dw 224                        ; 2 bytes; On the FAT12/16 volumes, this field indicates number of 32-byte directory entries in the root directory.
BPB_TotSec16   dw 2880                       ; 2 bytes; Total number of sectors 
BPB_Media      db 0xf0                       ; 1 bytes; 0xF0 is often used for non partitioned removable disks
BPB_FATSz16    dw 9                          ; 2 bytes; Number of sectors occupied by a FAT
BPB_SecPerTrk  dw 18                         ; 2 bytes; Number of sectors per track
BPB_NumHeads   dw 2                          ; 2 bytes; Number of heads
BPB_HiddSec    dd 0                          ; 4 bytes; Number of hidden physical sectors preceding the FAT volume
BPB_TotSec32   dd 0                          ; 4 bytes; Total number of sectors of FAT volume in new 32-bit field
BS_DrvNum      db 0                          ; 1 bytes; Drive number used by disk BIOS of IBM PC, This field is used in MS-DOS bootstrap, 
                                             ;          0x00 for floppy disk and 0x80 for fixed disk
BS_Reserved    db 0                          ; 1 bytes; should be zero
BS_BootSig     db 0x29                       ; 1 bytes; Extended boot signature (0x29)
BS_VolID       dd 0                          ; 4 bytes; Volume serial number used with BS_VolLab to track a volume on the removable storage.
BS_VOlLab      db 'SingOS     '              ; 11 bytes; Volume label
BS_FileSysType db 'FAT12   '                 ; 8 bytes; "FAT12   ", "FAT16   " or "FAT     "
; ------------------------------------------------------------------------------------------------------------------------------------------------------------
