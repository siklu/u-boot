/*
 * tmp421.c
 *
 *  Created on: Dec 19, 2017
 *      Author: edwardk
 *
 *      Written specially for Siklu PCB19x board
 */

#include <config.h>
#include <common.h>
#include <div64.h>
#include <fuse.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <linux/math64.h>
#include <thermal.h>
#include <i2c.h>

/* The TMP421 registers */
#define TMP421_STATUS_REG				0x08
#define TMP421_CONFIG_REG_1				0x09
#define TMP421_CONVERSION_RATE_REG		0x0B
#define TMP421_MANUFACTURER_ID_REG		0xFE
#define TMP421_DEVICE_ID_REG			0xFF

static const u8 TMP421_TEMP_MSB[4] = { 0x00, 0x01, 0x02, 0x03 };
static const u8 TMP421_TEMP_LSB[4] = { 0x10, 0x11, 0x12, 0x13 };

/* Flags */
#define TMP421_CONFIG_SHUTDOWN			0x40
#define TMP421_CONFIG_RANGE				0x04

/* Manufacturer / Device ID's */
#define TMP421_MANUFACTURER_ID			0x55
#define TMP421_DEVICE_ID				0x21

/*
 * Helper functions
 */

static u8 rtc_read(u8 reg) {
	uchar val;
	int old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_TEMP_SENS_BUS_NUM);
	val = (i2c_reg_read(CONFIG_SYS_I2C_TEMP_SENS_ADDR, reg));
	i2c_set_bus_num(old_bus);
	return val;
}

static void rtc_write(u8 reg, u8 val) {
	int old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_SYS_TEMP_SENS_BUS_NUM);
	i2c_reg_write(CONFIG_SYS_I2C_TEMP_SENS_ADDR, reg, val);
	i2c_set_bus_num(old_bus);
}

static int temp_from_s16(s16 reg) {
	/* Mask out status bits */
	int temp = reg & ~0xf;

	return (temp * 1000 + 128) / 256;
}

static int temp_from_u16(u16 reg) {
	/* Mask out status bits */
	int temp = reg & ~0xf;

	/* Add offset for extended temperature range. */
	temp -= 64 * 256;

	return (temp * 1000 + 128) / 256;
}


static int do_siklu_display_pcf421_regs(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_SUCCESS;
	const u8 pcf421_regs[] = {
			0,1,2,3,8,9, //
			0xa,0xb,0xf,0x10,0x11,0x12,0x13, //
			0x21,0x22,0x23,0xfe,0xff //
			};
	int i;


	for (i=0;i<sizeof(pcf421_regs);i++)
	{
		u8 val = rtc_read(pcf421_regs[i]);
		printf(" [%02x] 0x%02x\n",pcf421_regs[i], val );
	}

	return rc;
}


/*
 *
 */
static int do_siklu_read_temperature(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;
	u8 config = rtc_read(TMP421_CONFIG_REG_1);
	u16 local_temp_reg, remote1_temp_reg, remote2_temp_reg;
	int local_temp, remote1_temp, remote2_temp;

	(void)rtc_write; //prevent compil warning

	local_temp_reg = (rtc_read(TMP421_TEMP_MSB[0]) << 8)
			| rtc_read(TMP421_TEMP_LSB[0]);

	remote1_temp_reg = (rtc_read(TMP421_TEMP_MSB[1]) << 8)
			| rtc_read(TMP421_TEMP_LSB[1]);

	remote2_temp_reg = (rtc_read(TMP421_TEMP_MSB[2]) << 8)
			| rtc_read(TMP421_TEMP_LSB[2]);

	if (config & TMP421_CONFIG_RANGE) {
		local_temp = temp_from_u16(local_temp_reg);
		remote1_temp = temp_from_u16(remote1_temp_reg);
		remote2_temp = temp_from_u16(remote2_temp_reg);
	} else {
		local_temp = temp_from_s16(local_temp_reg);
		remote1_temp = temp_from_s16(remote1_temp_reg);
		remote2_temp = temp_from_s16(remote2_temp_reg);
	}

	printf(" Local %d, Remote1 %d, Remote2 %d\n", local_temp, remote1_temp,
			remote2_temp);

	return rc;
}

U_BOOT_CMD(temp, 1, 1, do_siklu_read_temperature,
		"Read TMP421 Remote and Local Temperature",
		" Read TMP421 Remote and Local Temperature");


U_BOOT_CMD(pcf421, 1, 1, do_siklu_display_pcf421_regs,
		"Display PCF421 registers",
		" Display PCF421 registers");
