/*
 * mvBoardSikluSysEepromCommon.c
 *
 *  Created on: Jul 12, 2016
 *      Author: edwardk
 */

#include <common.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <crc.h>
#include <malloc.h>

#include <spi_flash.h>
#include "siklu_board_system.h"

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

extern char *getenv_se(const char *name);
extern int setenv_se(const char *varname, const char *varvalue);
extern int saveenv_se(void);
extern int env_print_se(char *name, int flag);
extern void env_relocate_spec_se(void);

/* NOTE: these symbols are intentionally globalized */
extern struct spi_flash *env_flash;
extern int do_mem_md(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]);

DECLARE_GLOBAL_DATA_PTR;

int is_port_type_valid(char port)
{
    switch (port)
    {
    case 'c':
    case 'f':
    case '-':
        return 1;
        break;
    default:
        return 0;
        break;
    }
    return 0;
}

static int se_show_raw(char *size)
{
	char *const argv[] = {
		"md.b",
		SLINE(SCRATCH_ADDR),
		(size) ? size : "0x100"
	};
	int	rc;
	
	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_ENV_SPI_BUS,
				CONFIG_ENV_SPI_CS,
				CONFIG_ENV_SPI_MAX_HZ,
				CONFIG_ENV_SPI_MODE);
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

static int get_value(const char *name)
{
	int		rc = -1;

	if (name) {
		if (env_print_se((char *)name, 0) > 0)
			rc = 0;
	} else
		printf("%s: name is NULL\n", __func__);
	return rc;
}

static int set_value(const char *name, const char *value)
{
	int		rc = -1;

	if (name) {
		rc = setenv_se(name, value);
		if (rc == 0) {
			rc = saveenv_se();
			if (rc)
				printf("%s: saveenv_se()=%d\n", __func__, rc);
		} else
			printf("%s: setenv_se()=%d\n", __func__, rc);
	} else
		printf("%s: name is NULL\n", __func__);
	return rc;
}

#define ECMD(cmd)	(strcmp(argv[1], cmd) == 0)
static int do_maintenance_sys_serial_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int rc = CMD_RET_FAILURE;

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
	else
		printf("%s: unknown option %s\n", __func__, argv[1]);

	return rc;
}

U_BOOT_CMD(sseepro, 7, 1, do_maintenance_sys_serial_eeprom, "Read/Maintenance System Serial EEPROM raw data",
        "r [size] - read raw data of 'size' bytes. Default 'size' is 0x100\n\
        f - display entire EEPROM\n\
        w - primary format (NOT IMPLEMENTED)\n\
        e - erase (NOT IMPLEMENTED)\n\
        b - set baseband serial (NOT IMPLEMENTED)\n\
        m - set MAC (NOT IMPLEMENTED)\n\
        p - set Product Name (NOT IMPLEMENTED)\n\
        a - set assembly board type (NOT IMPLEMENTED)\n\
        get <name> - get value of EEPROM variable 'name'\n\
        set <name> [<value>] - set EEPROM variable 'name' to 'value'.\n\
                               If 'value' is skipped, 'name' is deleted.");
