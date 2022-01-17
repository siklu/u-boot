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

static int do_siklu_display_eth_mib_statistics(cmd_tbl_t * cmdtp, int flag,
		int argc, char * const argv[]) {
	int rc = CMD_RET_SUCCESS;
	uint32_t i;
	uint32_t mib_ptr = (uint32_t) (0x2188000 + 0x200); // offset eth1 mib counters

	printf(" Eth1 port MIB statistics:\n");
	for (i = mib_ptr; i <= (mib_ptr + 0x74); i += 4) {
		u32 val = readl((uint32_t*)i);
		printf("0x%x - 0x%08x\n", i, val);
	}

	i = 0x2188284;
	for (; i <= (mib_ptr + 0xe0); i += 4) {
		u32 val = readl((uint32_t*)i);
		printf("0x%x - 0x%08x\n", i, val);
	}

	return rc;
}

U_BOOT_CMD(seth_s, 1, 1, do_siklu_display_eth_mib_statistics,
		"Display eth port statistics", " Display eth port statistics");
