#ifndef __BOOTLOADER_H
#define __BOOTLOADER_H

void JumpToBootloader(void);

#ifndef BOOTLOADER_ADDR
#define BOOTLOADER_ADDR 0x1FFF0000UL
#endif  //BOOTLOADER_ADDR

#endif // __BOOTLOADER_H