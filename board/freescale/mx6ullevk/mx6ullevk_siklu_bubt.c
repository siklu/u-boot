/*
 * mx6ullevk_siklu_bubt.c
 *
 *  Created on: Feb 26, 2018
 *      Author: noama
 */


#include <common.h>
#include <command.h>
#include <version.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/compiler.h>
#include <dm.h>
#include <environment.h>
#include <malloc.h>
#include <search.h>
#include <errno.h>
#include <dm/device-internal.h>
#include <console.h>
#include <spi.h>
#include <spi_flash.h>

#include "siklu_def.h"
#include "siklu_api.h"


#define CONFIG_UBOOT_MIN_SIZE           0x4B000   /* min 300k*/
#define CONFIG_UBOOT_MAX_SIZE           0x100000  /* max 1M*/

#define CONFIG_SNOR_SIZE                0x200000  /* snor size 2M*/

extern struct spi_flash *siklu_get_env_flash(void);

/*
 * Extract arguments from bubt command line
 * argc, argv are the input arguments of bubt command line
 * loadfrom is pointer to the extracted argument: from where to load the u-boot bin file
 */
int fetch_bubt_cmd_args(int argc, char * const argv[], u32 *loadfrom)
{
	*loadfrom = 0;
	/* bubt */
	if (argc < 2) {
		copy_filename (net_boot_file_name, "u-boot.bin", sizeof(net_boot_file_name));
		printf("Using default filename \"u-boot.bin\" \n");
	}
	/* "bubt filename" */
	else {
		copy_filename (net_boot_file_name, argv[1], sizeof(net_boot_file_name));
	}

	return 0;
}

/*
 * Load u-boot bin file into ram from external device: tftp, usb or other devices
 * loadfrom specifies the source device: tftp, usb or other devices
 * (currently only tftp is supported )
 */
int fetch_uboot_file (int loadfrom)
{
	int filesize = 0;
	filesize = net_loop(TFTPGET);
	if (filesize == -1)
	{
		printf("Received file is wrong (file size error)\n");
		return 0;
	}
	return filesize;
}


/* Boot from SNOR flash */
/* Write u-boot image into the SNOR flash */
int sf_burn_uboot_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int filesize = 0;
	int rc = 0;
	u32 loadfrom = 0; /* 0 - from tftp */

	/* verify requested source is valid */
	if (fetch_bubt_cmd_args(argc, argv, &loadfrom) != 0)
		return 0;

	if ((filesize = fetch_uboot_file (loadfrom)) <= 0)
		return 0;

	if (filesize > CONFIG_UBOOT_MAX_SIZE) {
		printf("Boot image is too big. Maximum size is %d bytes\n", CONFIG_UBOOT_MAX_SIZE);
		return 0;
	}

	if (filesize < CONFIG_UBOOT_MIN_SIZE) {
		printf("Boot image is too small. Minimum size is %d bytes\n", CONFIG_UBOOT_MIN_SIZE);
		return 0;
	}

	if (1) {  // check image footer. See sdk_nxp/host-scripts/create_target_uboot.sh  add_footer4spi_nor_boot() function
		uint32_t footer_offs = load_addr + filesize - 100;
		char* footer=(char*)footer_offs;
		//printf(" Load address %lu, filesize %u, footer_offs %u\n", load_addr, filesize, footer_offs);
		//printf(" New uboot footer string %s\n", (char*)footer_offs);

		char* last_byte = (char*)(load_addr + filesize);
		*last_byte = 0; // protect from oversize

		if (!strstr(footer, "PCB19x UBOOT")) {
			printf("UBOOT image is wrong. No right footer\n");
			return 0;
		}
	}

	printf("\t[Done]\n");
	printf("Erasing SPI flash 0x%x - 0x%x: ", 0, CONFIG_SNOR_SIZE - CONFIG_ENV_SIZE);

	rc = spi_flash_erase(siklu_get_env_flash(), 0, CONFIG_SNOR_SIZE - CONFIG_ENV_SIZE);
	if (rc) {
		printf("BUBT SF area erase error rc %d\n", rc);
		return rc;
	}
	printf("\t[Done]\n");
	printf("Writing image to SNOR:");

	rc = spi_flash_write(siklu_get_env_flash(), 0, filesize,
			(char*) load_addr);
	if (rc)
		printf("\t[Fail]\n");
	else
		printf("\t[Done]\n");

	return 1;
}


U_BOOT_CMD(
		bubt,      2,    0,      sf_burn_uboot_cmd,
		"bubt	- Burn an image on the Boot Nand Flash.",
		"\n\t[file-name]\n"
		"\tBurn a binary image on the Boot Flash, default file-name is u-boot.bin .\n"
		"\tsource is tftp.\n"
);

