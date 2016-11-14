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

int siklu_control_sfp_led(int is_on) {
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
	mvSikluExtndrGpioSetVal(1, is_ena); // edikk TBD?
	return rc;
}

static int siklu_display_sfp_spd_data(void) {
	int rc = CMD_RET_SUCCESS;
	int sfp_exists;
#define SFP_DATA_SIZE 	95
	MV_U8 data[SFP_DATA_SIZE];
	int count;
	char temp[50];

	mvSikluCpuGpioGetVal(54, &sfp_exists);
	sfp_exists = !sfp_exists; // be careful - EXIST bit inverted

	if (!sfp_exists) {
		printf("  SFP Device isn't present\n");
		return rc; // return CMD_RET_SUCCESS here
	}

#define CONFIG_SFP_P3_BUS_NUM 1
#define CONFIG_SFP_P3_DEV_ADDR	0x50

	int old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SFP_P3_BUS_NUM);
	for (count = 0; count < SFP_DATA_SIZE; count++) {
		data[count] = i2c_reg_read(CONFIG_SFP_P3_DEV_ADDR, count);
	}
	i2c_set_bus_num(old_bus);
	// display data RAW
	for (count = 0; count < SFP_DATA_SIZE; count++) {
		if ((count % 0x10)==0)
			printf("\n[%02x] ",count);
		printf("%02x ", data[count]);
	}
	printf("\n\n\n");
	// display fields
	/* based on follow format
	SFP_ID_FLD(id, 0, 1),
	SFP_ID_FLD(connector, 2, 1),
	SFP_ID_FLD(transceiver, 3, 8),
	SFP_ID_FLD(br_nom, 12, 1),
	SFP_ID_FLD(vendor_name, 20, 16),
	SFP_ID_FLD(vendor_pn, 40, 16),
	SFP_ID_FLD(vendor_sn, 68, 16),
	 */

	printf("ID          %02x\n", data[0]);
	printf("Connector   %02x\n", data[1]);
	printf("Transceiver:\n");
	printf("\treserved                %02x.%02x\n", data[3],(data[4]>>5)&0x7);
	printf("\tSONET                   %02x.%02x\n", data[4]&0x1F,data[5]);
	printf("\tGiga-bit Eth            %02x\n", data[6]);
	printf("\tFibre chan link length  %02x\n", (data[7]>>4) &0xF);
	printf("\tFibre chan tr technol   %02x.%02x\n", (data[7]) &0xF, data[8]);
	printf("\tFibre chan tr media     %02x\n", data[9]);
	printf("\tFibre chan speed        %02x\n", data[10]);
	printf("BR Nominal  %dMbits/s\n", data[12]*100);

	memcpy(temp,data+20,16);
	temp[17] = 0;
	printf("Vendor Name %s\n",  temp);

	memcpy(temp,data+40,16);
	temp[17] = 0;
	printf("Vendor PN   %s\n",  temp);

	memcpy(temp,data+68,16);
	temp[17] = 0;
	printf("Vendor SN   %s\n",  temp);

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
		printf(" %s spd       - Display internal SPD\n", argv[0]);
		printf(" %s led [0/1] - Manual Set SFP LED Status\n", argv[0]);
		printf(" %s ena [0/1] - Manual Set SFP TX Enable Status\n", argv[0]);
		rc = CMD_RET_FAILURE;
		break;
	case 2 ... 5: {
		if (strncmp(argv[1], "stat", strlen("stat")) == 0) {
			rc = siklu_display_sfp_status();
		} else if (strncmp(argv[1], "spd", strlen("spd")) == 0) {
			rc = siklu_display_sfp_spd_data();
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
		"[stat/led/ena/spd] Control SFP3 Siklu Ethernet port (run command without param for help)");


