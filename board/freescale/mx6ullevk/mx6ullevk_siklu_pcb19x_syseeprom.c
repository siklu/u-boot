/*
 * mx6ullevk_siklu_pcb19x_syseeprom.c
 *
 *  Created on: Jan 11, 2018
 *      Author: edwardk
 */

#include <common.h>
#include <command.h>
#include <version.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/compiler.h>

#include <spi.h>
#include <spi_flash.h>

#include "siklu_def.h"
#include "siklu_api.h"

#ifndef CONFIG_ENV_SPI_MODE
# define CONFIG_ENV_SPI_MODE	SPI_MODE_3
#endif

#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE

#define CONFIG_ENV_SECT_SIZE	0x10000
#define CONFIG_ENV_OFFSET	(0x200000 - CONFIG_ENV_SECT_SIZE)
#define CONFIG_ENV_SIZE		CONFIG_ENV_SECT_SIZE

#define SCRATCH_ADDR		0x1000000

#define STRINGIFY(x)		#x
#define SLINE(num)		STRINGIFY(num)


/* NOTE: these symbols are intentionally globalized */
extern struct spi_flash *env_flash;


DECLARE_GLOBAL_DATA_PTR;

#if 0

static int se_show_raw(char *size)
{
	char *const argv[] = {
		"md.b",
		SLINE(SCRATCH_ADDR),
		(size) ? size : "0x100"
	};
	int	rc;

	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				CONFIG_SF_DEFAULT_CS,
				CONFIG_SF_DEFAULT_SPEED,
				CONFIG_SF_DEFAULT_MODE);
		if (!env_flash) {
			printf("%s: spi_flash_probe() failed\n", __func__);
			return -1;
		}
	}

	rc = spi_flash_read(env_flash, CONFIG_ENV_OFFSET,
				CONFIG_ENV_SIZE, (void *)SCRATCH_ADDR);
	if (rc == 0)
		do_mem_md(NULL, 0, 3, argv);
	else
		printf("%s: spi_flash_read()=%d\n", __func__, rc);
	return rc;
}

int se_init(void)
{
	static int	eeprom_ready;
	int		rc = 0;

	if (!eeprom_ready)
	{
		/* Borrow the field from the main environment to detect
		 * EEPROM CRC validity. gd->env_valid is definitely set here.
		 */
		gd->env_valid = 0;
		env_relocate_spec_se();
		if (!gd->env_valid)  /* No EEPROM env, save the default one. */
		{
			rc = saveenv_se();
			if (rc)
				printf("%s: saveenv_se()=%d\n", __func__, rc);
		}
		gd->env_valid = 1;
		eeprom_ready = 1;
	}
	return rc;
}






#endif // 0















#define ECMD(cmd)	(strcmp(argv[1], cmd) == 0)
static int do_maintenance_sys_serial_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc = CMD_RET_FAILURE;

#if 0

	if (argc == 1)
		return CMD_RET_USAGE;

	if (ECMD("r"))
		rc = se_show_raw(argv[2]);
	else if (ECMD("f"))
		rc = (env_print_se(NULL, 0) > 0) ? CMD_RET_SUCCESS :
							CMD_RET_FAILURE;
	else if (ECMD("get"))
		rc = get_value(argv[2]);
	else if (ECMD("set"))
		rc = set_value(argv[2], argv[3]);
	else if  (ECMD("m")) { // store mac in form as a continuous string without a colon inside
	    char* user_mac = argv[2];
	    char new_mac[30];
	    if (strlen(user_mac) != 12){
	        printf("Wrong MAC string %s, len %d\n", user_mac, strlen(user_mac));
	        return CMD_RET_FAILURE;
	    }
	    sprintf(new_mac,"%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", user_mac[0], user_mac[1],user_mac[2],user_mac[3],
	            user_mac[4],user_mac[5], user_mac[6],user_mac[7],
	            user_mac[8],user_mac[9],user_mac[10],user_mac[11]);

	    rc = set_value("SE_mac", new_mac);

	}
	else
		printf("%s: unknown option %s\n", __func__, argv[1]);

#else
	printf("TBD\n");
#endif // 0

	return rc;
}







U_BOOT_CMD(sseepro, 7, 1, do_maintenance_sys_serial_eeprom, "Read/Maintenance System Serial EEPROM raw data",
        "r [size] - read raw data of 'size' bytes. Default 'size' is 0x100\n\
        f - display entire EEPROM\n\
        w - primary format (NOT IMPLEMENTED)\n\
        e - erase (NOT IMPLEMENTED)\n\
        b - set baseband serial (NOT IMPLEMENTED)\n\
        m - set MAC in form as a continuous string without a colon inside\n\
        p - set Product Name (NOT IMPLEMENTED)\n\
        a - set assembly board type (NOT IMPLEMENTED)\n\
        get <name> - get value of EEPROM variable 'name'\n\
        set <name> [<value>] - set EEPROM variable 'name' to 'value'.\n\
                               If 'value' is skipped, 'name' is deleted.");



