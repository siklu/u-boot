/*
 * mvSikluSfpModuleControl.c
 *
 *  Created on: Jul 19, 2016
 *      Author: edwardk
 */

#include <common.h>
#include <command.h>
#include "mvCommon.h"
#include "mvOs.h"
#include <siklu_api.h>
#include <i2c.h>

static int siklu_display_sfp_status(void) {
	int rc = CMD_RET_SUCCESS;
	int sfp_exists, sfp_fault, sfp_los;
	// printf(" %s() Called\n",__func__);

	// # display MPP54 SFP EXISTS Status
	mvSikluCpuGpioGetVal(54, &sfp_exists);
	sfp_exists = !sfp_exists; // be careful - EXIST bit inverted
	// # display MPP55 SFP FAULT Status
	mvSikluCpuGpioGetVal(55, &sfp_fault);
	// # display MPP43 SFP LOS Status
	mvSikluCpuGpioGetVal(43, &sfp_los);

	printf("\tEXISTS %d, FAULT %d, LOS %d\n", sfp_exists, sfp_fault, sfp_los);
	return rc;
}

static int siklu_control_sfp_led(int is_on) {
	int rc = CMD_RET_SUCCESS;
	// printf(" %s() Called %d\n",__func__, is_on);

	// # set CPU MPP50 LED
	// be careful - '1' turns led off
	mvSikluCpuGpioSetVal(50, !is_on);

	return rc;
}

static int siklu_control_sfp_tx_ena(int is_ena) {
	int rc = CMD_RET_SUCCESS;
	// printf(" %s() Called %d\n",__func__, is_ena);

	// set I2C Extender PCA9557   bit IO1
	mvSikluExtndrGpioSetVal(1, is_ena);
	return rc;
}

/*
 *
 */
static int do_siklu_sfp3_control(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) //
{
	int rc = CMD_RET_SUCCESS;
	static int is_configured = 0;

	if (!is_configured) {
		// config all GPIO here
		// in - MPP43, MPP54, MPP55
		mvSikluCpuGpioSetDirection(43, 0);
		mvSikluCpuGpioSetDirection(54, 0);
		mvSikluCpuGpioSetDirection(55, 0);

		// out - MPP50; IO extender IO1
		mvSikluCpuGpioSetDirection(50, 1);
		mvSikluExtndrGpioSetDirection(1, 1);  // PCA9557 IIC IO extender

		is_configured = 1;
	}

	switch (argc) {
	case 1: // no args, print help and exit
		printf(" %s stat      - Print SFP status\n", argv[0]);
		printf(" %s led [0/1] - Manual Set SFP LED Status\n", argv[0]);
		printf(" %s ena [0/1] - Manual Set SFP TX Enable Status\n", argv[0]);
		rc = CMD_RET_FAILURE;
		break;
	case 2 ... 5: {
		if (strncmp(argv[1], "stat", strlen("stat")) == 0) {
			rc = siklu_display_sfp_status();
		} else if ((argc == 3)
				&& (strncmp(argv[1], "led", strlen("led")) == 0)) {
			int is_on = !!simple_strtoul(argv[2], NULL, 10);
			rc = siklu_control_sfp_led(is_on);
		} else if ((argc == 3)
				&& (strncmp(argv[1], "ena", strlen("ena")) == 0)) {
			int is_ena = !!simple_strtoul(argv[2], NULL, 10);
			rc = siklu_control_sfp_tx_ena(is_ena);
		} else {
			printf(" Unknown command. EXIT\n");
			return CMD_RET_USAGE;
		}
	}
		break;
	}

	return rc;
}

U_BOOT_CMD(ssfp, 7, 1, do_siklu_sfp3_control,
		"Control SFP3 Siklu Ethernet port", //
		"[stat/led/ena] Control SFP3 Siklu Ethernet port (run command without param for help)");
