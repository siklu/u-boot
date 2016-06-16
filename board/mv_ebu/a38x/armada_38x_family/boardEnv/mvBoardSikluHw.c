/*
 * mvBoardSikluHw.c
 *
 *  Created on: Jun 9, 2016
 *      Author: edwardk
 */

#include <common.h>
#include <command.h>
#include "mvCommon.h"
#include "mvOs.h"
#include <siklu_api.h>
#include <i2c.h>

#define	MV_GPP_IN	0xFFFFFFFF	/* GPP input */
#define MV_GPP_OUT	0		/* GPP output */

extern MV_STATUS mvGppValueSet(MV_U32 group, MV_U32 mask, MV_U32 value);
extern MV_STATUS mvGppTypeSet(MV_U32 group, MV_U32 mask, MV_U32 value);
extern MV_U32 mvGppValueGet(MV_U32 group, MV_U32 mask);
/*
 *
 */
int mvSikluCpuGpioSetVal(int gppNum, int val) {
	int rc = 0;
	int group = 0;

	if (gppNum >= 32) {
		group = 1;
		gppNum -= 32;
	}

	val = !!val;
	mvGppTypeSet(group, 1 << gppNum, (MV_GPP_OUT & (1 << gppNum))); // configure gppX as output
	mvGppValueSet(group, 1 << gppNum, (val << gppNum));

	return rc;

}
/*
 *
 */
int mvSikluCpuGpioSetDirection(int gppNum, int isOutput) {
	int rc = 0;
	int group = 0;

	if (gppNum >= 32) {
		group = 1;
		gppNum -= 32;
	}

	if (isOutput) {
		mvGppTypeSet(group, 1 << gppNum, (MV_GPP_OUT & (1 << gppNum))); // configure gppX as output
	} else {
		mvGppTypeSet(group, 1 << gppNum, (MV_GPP_IN & (1 << gppNum))); // configure gppX as input
	}

	return rc;

}
/*
 *
 */
int mvSikluCpuGpioGetVal(int gppNum, int* val) {
	int rc = 0;
	int group = 0;

	if (gppNum >= 32) {
		gppNum -= 32;
		group = 1;
	}

	*val = !!mvGppValueGet(group, 1 << gppNum);
	return rc;
}

#define CONFIG_PCA9557_BUS_NUM 1
#define CONFIG_PCA9557_DEV_ADDR	0x18

#define PCA9557_INPUT_PORT_REG		0x00
#define PCA9557_OUTPUT_PORT_REG		0x01
#define PCA9557_POLAR_INVERT_REG	0x02
#define PCA9557_CONFIG_REG			0x03

#define PCA9557_GPIO_POE_PARS			0 // input
#define PCA9557_GPIO_SFP_P3_TX_DIS		1 // output
#define PCA9557_GPIO_SFP_P2_TX_DIS		2 // output
#define PCA9557_GPIO_WG0_DIS			3 // output
#define PCA9557_GPIO_WG1_DIS			4 // output
#define PCA9557_GPIO_WLAN_DIS			5 // output
#define PCA9557_GPIO_BLE_DIS			6 // output
#define PCA9557_GPIO_VHV_DIS			7 // output

/*
 * I/O extender
 0	PoE_Pairs		In		PoE Pairs status (0: 2 pairs, 1: 4 pairs)
 1	PSE_P3_RST_n	Out		SFP_P3_TX_DIS	PSE ports 3 Rest or SFP port 3 Tx Disable
 2	PSE_P2_RST_n	Out		SFP_P2_TX_DIS	PSE ports 2 Rest or SFP port 2 Tx Disable
 3	WG0_DISABLE_n	Out		WiGig Modem 0 Chip Disable (active low)
 4	WG1_DISABLE_n	Out		WiGig Modem 1 Chip Disable (active low)
 5	WLAN_Disable	Out		WLAN (mini-PCIe) Disable (active low)
 6	BLE_RST_n		Out		BLE chip Reset (active low)
 7	VHV enable		Out		Enable 1.8V for eFuse burning
 */
static int mvSikluExtndrGpioConf(void) {
	int rc = 0, count;

	mvSikluExtndrGpioSetDirection(0, 0);
	for (count = 1; count <= 7; count++) {
		mvSikluExtndrGpioSetDirection(count, 1);
	}

	int old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);

	i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, PCA9557_POLAR_INVERT_REG, 0); // disable polarity inversion for all inputs
	i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, PCA9557_OUTPUT_PORT_REG, 0x0); // set GPIO[7..1] be output val = 0 (all in reset)

	i2c_set_bus_num(old_bus);
	return rc;
}
/*
 * Configure SoC MPP pins
 */
static int mvSikluCpuGpioConf(void) {
	int rc = 0;

	// configure MPP6 GPHY port2 reset, active low output
	mvSikluCpuGpioSetDirection(6, 1);
	mvSikluCpuGpioSetVal(6, 1);// edikk change to reset

	// configure MPP12 Power Led Yellow output
	mvSikluCpuGpioSetDirection(12, 1);

	// configure MPP13 SFP P2 exists input
	mvSikluCpuGpioSetDirection(13, 0);
	// configure MPP14 SFP P2 FAULT input
	mvSikluCpuGpioSetDirection(14, 0);
	// configure MPP15-18  HW ID input
	mvSikluCpuGpioSetDirection(15, 0);
	mvSikluCpuGpioSetDirection(16, 0);
	mvSikluCpuGpioSetDirection(17, 0);
	mvSikluCpuGpioSetDirection(18, 0);

	// configure MPP21 Powe LED Green output
	mvSikluCpuGpioSetDirection(21, 1);

	// configure MPP43 SFP P3 LOS	input
	mvSikluCpuGpioSetDirection(43, 0);

	// configure MPP44 WIGIG0 CHIP reset active low output
	mvSikluCpuGpioSetDirection(44, 1);
	mvSikluCpuGpioSetVal(44, 1); // edikk change to reset
	// configure MPP47 WIGIG1 CHIP reset active low output
	mvSikluCpuGpioSetDirection(47, 1);
	mvSikluCpuGpioSetVal(47, 1); // edikk change to reset
	// configure MPP48 RF LED Green output
	mvSikluCpuGpioSetDirection(48, 1);
	// configure MPP49 RF LED Yellow  	output
	mvSikluCpuGpioSetDirection(49, 1);

	// configure MPP50 PHY Port3 Reset output
	mvSikluCpuGpioSetDirection(50, 1);
	mvSikluCpuGpioSetVal(50, 1); // edikk change to reset
	// configure MPP51 WIGIG2 CHIP reset active low output
	mvSikluCpuGpioSetDirection(51, 1);
	mvSikluCpuGpioSetVal(51, 1); // edikk change to reset
	// configure MPP52 RST Factory default input
	mvSikluCpuGpioSetDirection(52, 0);

	// configure MPP53  PHY P1 Reset output
	mvSikluCpuGpioSetDirection(53, 1);
	mvSikluCpuGpioSetVal(53, 1); // edikk change to reset
	// configure MPP54  SFP P3 Exists  input
	mvSikluCpuGpioSetDirection(54, 0);
	// configure MPP55 	SFP P3 Fault input
	mvSikluCpuGpioSetDirection(55, 0);

	return rc;
}

/*
 *  IIC PCA9557 GPI Extender for 8 IO
 */
int mvSikluExtndrGpioSetVal(int gpio, int val) {
	int rc = 0;
	if (gpio >= 8)
		return -1;

	int old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);
	MV_U8 reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR,
	PCA9557_OUTPUT_PORT_REG);
	if (val)
		reg_val |= (1 << gpio);
	else
		reg_val &= ~(1 << gpio);

	i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, PCA9557_OUTPUT_PORT_REG, reg_val);
	i2c_set_bus_num(old_bus);

	return rc;
}
/*
 *  IIC PCA9557 GPI Extender for 8 IO
 */
int mvSikluExtndrGpioSetDirection(int gpio, int isOutput) {
	int rc = 0;
	if (gpio >= 8)
		return -1;

	int old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);
	MV_U8 reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR,
	PCA9557_CONFIG_REG);
	if (!isOutput)
		reg_val |= (1 << gpio);
	else
		reg_val &= ~(1 << gpio);

	i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, PCA9557_CONFIG_REG, reg_val);
	i2c_set_bus_num(old_bus);

	return rc;
}
/*
 *  IIC PCA9557 GPI Extender for 8 IO
 */
int mvSikluExtndrCpuGpioGetVal(int gpio, int* val) {
	int rc = 0;
	if (gpio >= 8)
		return -1;

	int old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);
	MV_U8 reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR,
	PCA9557_INPUT_PORT_REG);
	if (reg_val & (1 << gpio))
		*val = 1;
	else
		*val = 0;
	i2c_set_bus_num(old_bus);
	return rc;
}

/* ##########################################################################################################
 ########################################################################################################## */
static int do_siklu_pca9557_access(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) //
{
	int rc = CMD_RET_SUCCESS;
	MV_U8 reg_val;
	MV_U8 reg_addr;

	int old_bus = i2c_get_bus_num();
	i2c_set_bus_num(CONFIG_PCA9557_BUS_NUM);

	switch (argc) {
	case 1: // no args, print all registers
	{
		int count;
		for (count = PCA9557_INPUT_PORT_REG; count <= PCA9557_CONFIG_REG;
				count++) {
			reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR, count);
			printf(" 0x%x: val 0x%x\n", count, reg_val);
		}
	}
		break;
	case 2: // read specif register
		reg_addr = simple_strtoul(argv[1], NULL, 16);
		reg_val = i2c_reg_read(CONFIG_PCA9557_DEV_ADDR, reg_addr);
		printf(" 0x%x: val 0x%x\n", reg_addr, reg_val);
		break;
	case 3: // write specif register
		reg_addr = simple_strtoul(argv[1], NULL, 16);
		reg_val = simple_strtoul(argv[2], NULL, 16);
		i2c_reg_write(CONFIG_PCA9557_DEV_ADDR, reg_addr, reg_val);
		break;
	}

	i2c_set_bus_num(old_bus);
	return rc;
}
/*
 *
 */
int mvSikluHwResetCntrl(SKL_MODULE_RESET_CNTRL_E dev, int isEna) {
	int rc = 0;

	switch (dev) {
	case SKL_WIGIG0_RF_RESET:
		mvSikluExtndrGpioSetVal(PCA9557_GPIO_WG0_DIS, !isEna);
		break;
	case SKL_WIGIG1_RF_RESET:
		mvSikluExtndrGpioSetVal(PCA9557_GPIO_WG1_DIS, !isEna);
		break;
	case SKL_WIFI_RESET:
		mvSikluExtndrGpioSetVal(PCA9557_GPIO_WLAN_DIS, !isEna);
		break;
	case SKL_BLE_RESET:
		mvSikluExtndrGpioSetVal(PCA9557_GPIO_BLE_DIS, !isEna);
		break;
	case SKL_WIGIG0_CHIP_RESET:
		mvSikluCpuGpioSetVal(44, !isEna);
		break;
	case SKL_WIGIG1_CHIP_RESET:
		mvSikluCpuGpioSetVal(47, !isEna);
		break;
	case SKL_WIGIG2_CHIP_RESET:
		mvSikluCpuGpioSetVal(51, !isEna);
		break;
	case SKL_GPHY_0_RESET:
		mvSikluCpuGpioSetVal(53, !isEna);
		break;
	case SKL_GPHY_1_RESET:
		mvSikluCpuGpioSetVal(6, !isEna);
		break;
	case SKL_GPHY_2_RESET:
		mvSikluCpuGpioSetVal(50, !isEna);
		break;
	default:
		break;
	}

	return rc;
}

int arch_early_init_r(void) {

	// configure IIC GPIO Extender
	mvSikluExtndrGpioConf();

	// configure CPU GPIO
	mvSikluCpuGpioConf();

	// edikk - follow part remove after debug. we do not need PCIe enabled by default
	mvSikluHwResetCntrl(SKL_WIGIG0_RF_RESET, 0); // edikk enable WIGIG radio ???
	mvSikluHwResetCntrl(SKL_WIGIG1_RF_RESET, 0); // edikk enable WIGIG radio ???
	mvSikluHwResetCntrl(SKL_WIFI_RESET, 0);
	mvSikluHwResetCntrl(SKL_BLE_RESET, 1);

	mvSikluHwResetCntrl(SKL_WIGIG0_CHIP_RESET, 0);
	mvSikluHwResetCntrl(SKL_WIGIG1_CHIP_RESET, 0);
	mvSikluHwResetCntrl(SKL_WIGIG2_CHIP_RESET, 0);

	mvSikluHwResetCntrl(SKL_GPHY_0_RESET, 0);
	mvSikluHwResetCntrl(SKL_GPHY_1_RESET, 0);
	mvSikluHwResetCntrl(SKL_GPHY_2_RESET, 0);



	udelay(10000);

	return 0;
}

/*
 *
 *
 *
 *
 *
 *
 */
static int do_siklu_pca9557_config(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) //
{
	int rc = CMD_RET_SUCCESS;
	mvSikluExtndrGpioConf();

	return rc;
}

U_BOOT_CMD(spca9557, 7, 1, do_siklu_pca9557_access,
		"Read/Write PCA9557 IIC Extender", //
		"[reg] [val*] Read/Write PCA9557 IIC Extender");

U_BOOT_CMD(spca9557c, 7, 1, do_siklu_pca9557_config,
		"Config PCA9557 IIC Extender to default values", //
		"Config PCA9557 IIC Extender to default values");

