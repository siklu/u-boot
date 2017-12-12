/*
 * mx6ullevk_siklu_soho.c
 *
 *  Created on: Dec 12, 2017
 *      Author: edwardk
 */

#include <common.h>
#include <linux/ctype.h>
#include <nand.h>
#include <command.h>
#include <version.h>
#include <libfdt.h>
#include <fdt_support.h>

#include <miiphy.h>

#include "siklu_def.h"
#include "siklu_api.h"


/*
 *
 */
static int do_siklu_soho_ver_read(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;
#define SOHO_NXP_CPU_PORT 	0
#define SWITCH_IDENT_REG	3
	u16 val;

	int ret = siklu_88e639x_reg_read(SOHO_NXP_CPU_PORT, SWITCH_IDENT_REG, &val);
	if (ret == 0)
	{
		printf( " 0x%04X\n", val);
	}
	else {
		printf( " Read ERROR %d\n", ret);
	}

	return rc;
}









U_BOOT_CMD(sohov, 1, 1, do_siklu_soho_ver_read,
		"Read SOHO Product Number & revision",
		" Read SOHO Product Number & revision");
