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

#define SCRATCH_ADDR		0x85000000

#define STRINGIFY(x)		#x
#define SLINE(num)		STRINGIFY(num)

/* NOTE: these symbols are intentionally globalized */
// extern struct spi_flash *env_flash;
DECLARE_GLOBAL_DATA_PTR;

static int se_show_raw(char *_size) {

	int rc;
	char* buf = (char*) SCRATCH_ADDR;
	char cmd[100];
	int size = 0x100;

	if (_size)
		size = simple_strtoul(_size, NULL, 10);

	rc = siklu_sf_sys_eeprom_read(buf, size);
	if (rc != 0) {
		printf("%s: Error %d, line %d\n", __func__, rc, __LINE__);
		return CMD_RET_FAILURE;
	}
	sprintf(cmd, "md.b 0x%p %d", buf, size);
	rc = run_command(cmd, 0);
	if (rc != 0) {
		printf("%s: Error %d, line %d\n", __func__, rc, __LINE__);
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

/*
 *
 */
static int se_erase(void) {
	int rc = siklu_sf_sys_eeprom_erase();
	return rc;
}

/*
 *	Write to SYSEEPROM area string instead valid SYSEEPROM data
 */
static int se_test(void) {
	int rc;
	char* buf = (char*) 0x80000000;

	memset(buf, 0, 0x100);
	sprintf(buf, "Hello world");
#if 1
	rc = siklu_sf_sys_eeprom_write(buf, sizeof(buf));
#else
	{
		char cmd[100];
		sprintf(cmd, "sf update 0x%x 0x%x 0x%x", 0x80000000,
				SIKLU_SYSEEPROM_SF_OFFS, strlen(buf) + 10);
		rc = run_command(cmd, 0);
	}
#endif
	return rc;
}

static int se_init(void) {
	int rc = siklu_syseeprom_init();
	if (rc != 0) {
		rc = siklu_syseeprom_restore_default();
		if (rc != 0) {
			printf("Error restore default SYSEEPROM\n");
			return -1;
		}
		// second attempt for init after restore
		rc = siklu_syseeprom_init();
		if (rc != 0) {
			printf("Error init SYSEEPROM\n");
			return -1;
		}
	}
	return rc;
}


/*
 *
 */
static int se_update_storage(void) {
	int rc = 0;
	rc = siklu_syseeprom_udate();
	if (rc != 0) {
		printf("Error update SYSEEPROM\n");
		return -1;
	}
	return rc;
}



static int se_get_value(const char *key) {
	int rc = -1;

	if (key) {
		char val[KEY_VAL_FIELD_SIZE];
		memset(val, 0, sizeof(val));
		rc = siklu_syseeprom_get_val(key, val);
		if (rc == 0)
			printf(" %s\n", val);
		else {
			printf(" No key \"%s\" in database\n", key);
		}
	} else
		printf("%s: key is NULL\n", __func__);
	return rc;
}

static int se_set_value(const char *name, const char *value) {
	int rc = -1;

	if (name) {
		rc = siklu_syseeprom_set_val(name, value);
		if (rc == 0) {
			/*
			 rc = saveenv_se();
			 if (rc)
			 printf("%s: saveenv_se()=%d\n", __func__, rc);
			 */
		} else
			printf("%s: setenv_se()=%d\n", __func__, rc);
	} else
		printf("%s: name is NULL\n", __func__);

	return rc;
}

/*
 *
 */
#define ECMD(cmd)	(strcmp(argv[1], cmd) == 0)
static int do_maintenance_sys_serial_eeprom(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[]) {
	int rc = CMD_RET_FAILURE;

	if (argc == 1)
		return CMD_RET_USAGE;

	if (ECMD("r"))
		rc = se_show_raw(argv[2]);
	else if (ECMD("e"))
		rc = se_erase();
	else if (ECMD("t"))
		rc = se_test();
	else if (ECMD("i"))
		rc = se_init();
	else if (ECMD("u"))
		rc = se_update_storage();
	else if ((ECMD("d")) || (ECMD("f")))
		rc = siklu_syseeprom_display();
	else if (ECMD("get")) {
		rc = se_get_value(argv[2]);
	} else if (ECMD("set"))
		rc = se_set_value(argv[2], argv[3]);
	else {
		printf("%s: unknown option %s\n", __func__, argv[1]);
	}

	if (rc != 0)
		rc = CMD_RET_FAILURE;

#if 0
	else if (ECMD("f"))
	rc = (env_print_se(NULL, 0) > 0) ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
	else if (ECMD("get"))
	rc = get_value(argv[2]);
	else if (ECMD("set"))
	rc = set_value(argv[2], argv[3]);
	if (argc == 1)
	return CMD_RET_USAGE;

	==if (ECMD("r"))
	==rc = se_show_raw(argv[2]);
	== else if (ECMD("f"))
	==rc = (env_print_se(NULL, 0) > 0) ? CMD_RET_SUCCESS :

	else if (ECMD("m")) { // store mac in form as a continuous string without a colon inside
		char* user_mac = argv[2];
		char new_mac[30];
		if (strlen(user_mac) != 12) {
			printf("Wrong MAC string %s, len %d\n", user_mac, strlen(user_mac));
			return CMD_RET_FAILURE;
		}
		sprintf(new_mac,"%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", user_mac[0], user_mac[1],user_mac[2],user_mac[3],
				user_mac[4],user_mac[5], user_mac[6],user_mac[7],
				user_mac[8],user_mac[9],user_mac[10],user_mac[11]);

		rc = set_value("SE_mac", new_mac);

	}

#endif // 0

	return rc;
}

U_BOOT_CMD(sseepro, 7, 1, do_maintenance_sys_serial_eeprom,
		"Read/Maintenance System Serial EEPROM raw data",
		"r [size] - read raw data of 'size' bytes. Default 'size' is 0x100\n\
        f - display entire EEPROM\n\
        w - primary format (NOT IMPLEMENTED use instead 'e' and 'i')\n\
        e - erase \n\
        t - test \n\
        i - init \n\
        d - display \n\
        u - Write latest data to storage\n\
		get <name> - get value of EEPROM variable 'name'\n\
        set <name> [<value>] - set EEPROM variable 'name' to 'value'.\n\
                               If 'value' is skipped, 'name' is deleted.");

