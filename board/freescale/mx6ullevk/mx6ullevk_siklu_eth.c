/*
 * mx6ullevk_siklu_eth.c
 *
 *  Created on: Dec 13, 2017
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

#include "siklu_def.h"
#include "siklu_api.h"




static int do_siklu_display_eth_mib_statistics(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;


	return rc;
}






U_BOOT_CMD(seth_s, 1, 1, do_siklu_display_eth_mib_statistics,
		"Display eth port statistics",
		" Display eth port statistics");
