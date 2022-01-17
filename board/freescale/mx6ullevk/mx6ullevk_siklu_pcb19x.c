/*
 * mx6ullevk_siklu_pcb19x.c
 *
 *  Created on: Aug 28, 2017
 *      Author: edwardk
 *
 *
 *      The file based on
 *      	main_mrv/sdk_armada/infra/u-boot-2013.01-2015_T1.0/board/mv_ebu/a38x/armada_38x_family/boardEnv/mvBoardSikluFlashBoot.c
 *      file
 *
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

// #include <siklu_api.h>
// #include "siklu_board_system.h"

// #define BOOT_DEBUG

static int do_siklu_soc_reg_read(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;

	if (argc > 1) {
		uint32_t reg_val;
		ulong reg_addr;
		if (strict_strtoul(argv[1], 16, &reg_addr) < 0)
			return CMD_RET_USAGE;

		reg_val = readl((uint32_t*)reg_addr);
		printf(" - 0x%x\n",reg_val);
		rc = CMD_RET_SUCCESS;
	}
	return rc;
}

static int do_siklu_soc_reg_write(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;

	if (argc > 2) {
		ulong reg_val, reg_addr;
		if (strict_strtoul(argv[1], 16, &reg_addr) < 0)
			return CMD_RET_USAGE;
		if (strict_strtoul(argv[2], 16, &reg_val) < 0)
			return CMD_RET_USAGE;

		writel(reg_val, (uint32_t*)reg_addr);
		rc = CMD_RET_SUCCESS;
	}
	return rc;
}



U_BOOT_CMD(sregr, 5, 0, do_siklu_soc_reg_read, "Read SoC Register",
		" [addr] Read SoC Register");

U_BOOT_CMD(sregw, 5, 0, do_siklu_soc_reg_write, "Write SoC Register",
		" [addr] [val] - Write SoC Register");




