#ifndef __SIKLU_COMMON_BOOT_H__
#define __SIKLU_COMMON_BOOT_H__

/**
 * Like booti <image_address> - <dtb_address>
 * @param image_address 
 * @param dtb_address 
 * @return < 0 on failure, should not return on success.
 */
int load_kernel_image(uintptr_t image_address, uintptr_t dtb_address);

/**
 * Configure boot arguments for the next boot.
 * @param bootargs boot arguments string.
 */
void setup_bootargs(const char *bootargs);

#endif