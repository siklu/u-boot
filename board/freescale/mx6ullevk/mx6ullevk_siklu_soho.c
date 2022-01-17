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
#include "../drivers/net/fec_mxc.h"

#include <asm/io.h>
#include <linux/errno.h>
#include <linux/compiler.h>

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/sys_proto.h>

#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/io.h>
#include <netdev.h>
#include <fsl_esdhc.h>
#include <linux/sizes.h>
#include <mmc.h>
#include <phy.h>
#include <linux/mdio.h>

#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6ull_pins.h>

#include "siklu_def.h"
#include "siklu_api.h"
#include "cpld_reg.h"

#define msleep(a) udelay(a * 1000)

/*
 * SOHO Access
 */
int siklu_88e639x_reg_read(u8 port, u8 reg, u16* val) {
	int rc = 0;

	const char *devname;

	// connect SOHO MDIO
	siklu_mdio_bus_connect(SIKLU_MDIO_BUS0);

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return -1;
	}

	if (miiphy_read(devname, port, reg, val) != 0) {
		printf("%s() ERROR read port 0x%x, reg 0x%x\n", __func__, port, reg);
		return -1;
	}

	// return bus to 10G PHY
	siklu_mdio_bus_connect(SIKLU_MDIO_BUS0);

	return rc;
}
/*
 *  SOHO Access
 */
int siklu_88e639x_reg_write(u8 port, u8 reg, u16 val) {
	int rc = 0;

	/* use current device  */
	const char *devname;

	// connect SOHO MDIO
	siklu_mdio_bus_connect(SIKLU_MDIO_BUS0);

	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return -1;
	}

	if (miiphy_write(devname, port, reg, val) != 0) {
		printf("%s() ERROR write port 0x%x, reg 0x%x\n", __func__, port, reg);
		return -1;
	}
	siklu_mdio_bus_connect(SIKLU_MDIO_BUS0);
	return rc;
}

/*
 *
 */
static int soho_display_page(u8 port) {
	int rc = 0;

	int i;
	//u8 reg;

	switch (port) {
	case 0 ... 0xA: {
		const int port_reg_map[] = // valid regs address
				{ 0, 1, 2, 3, 4, 5, 6, //
						7, 8, 9, 0xa, 0xb, //
						0xc, 0xd, 0xe, 0xf, 0x16, //
						0x17, 0x18, 0x19, 0x1b, //
						0x1c, 0x1e, 0x1f, -1 };
		for (i = 0; port_reg_map[i] != -1; i++) {
			u16 val;
			rc = siklu_88e639x_reg_read(port, port_reg_map[i], &val);
			if (rc != 0) {
				printf("Port %x, Reg %x read error\n", port, port_reg_map[i]);
				break;
			}
			printf("[%02x]    - 0x%04x\n", port_reg_map[i], val);
		}

	}
		break;
	case 0x1b: {
		const int switch_global1_regs[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
				0xa, 0xb, 0xc, 0xd, 0xe, 0xf, //
				0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, -1 };
		for (i = 0; switch_global1_regs[i] != -1; i++) {
			u16 val;
			rc = siklu_88e639x_reg_read(port, switch_global1_regs[i], &val);
			if (rc != 0) {
				printf("Port %x, Reg %x read error\n", port,
						switch_global1_regs[i]);
				break;
			}
			printf("[%02x]    - 0x%04x\n", switch_global1_regs[i], val);
		}
	}
		break;
	case 0x1c: {
		const int switch_global2_regs[] = { //
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
						0xa, 0xb, 0xc, 0xd, 0xe, 0xf, //
						0x12, 0x13, 0x14, 0x15, 0x16, 0x17, //
						0x18, 0x19, //
						0x1a, 0x1b, 0x1c, 0x1d, 0x1f, -1 };
		for (i = 0; switch_global2_regs[i] != -1; i++) {
			u16 val;
			rc = siklu_88e639x_reg_read(port, switch_global2_regs[i], &val);
			if (rc != 0) {
				printf("Port %x, Reg %x read error\n", port,
						switch_global2_regs[i]);
				break;
			}
			printf("[%02x]    - 0x%04x\n", switch_global2_regs[i], val);
		}
	}
		break;
	default:
		printf("Unknown port %x\n", port);
		break;
	}

	return rc;
}
/*
 *
 */
static int soho_display_register(u8 port, u8 reg) {
	int rc = 0;
	u16 val = 0x5555;

	rc = siklu_88e639x_reg_read(port, reg, &val);
	if (rc == 0) {
		printf("Port %x, Reg %x, Val 0x%x\n", port, reg, val);
	} else
		printf("Port %x, Reg %x read error\n", port, reg);
	return rc;
}

/*
 *
 */
static int soho_write_register(u8 port, u8 reg, u16 val) {
	int rc = 0;

	rc = siklu_88e639x_reg_write(port, reg, val);

	return rc;
}

/*
 *
 */
int siklu_soho_power_up_init(void) {
	int rc = 0;
	T_CPLD_LOGIC_MODEM_LEDS_CTRL_REGS reset_reg;

	// Put SOHO to reset
	reset_reg.uint8 = siklu_cpld_read(R_CPLD_LOGIC_MODEM_LEDS_CTRL);
	if (rc != 0) {
		printf("%s() ERROR, line %d\n", __func__, __LINE__);
		return rc;
	}
	reset_reg.s.cfg_switch_rst_n = 0;
	siklu_cpld_write(R_CPLD_LOGIC_MODEM_LEDS_CTRL, reset_reg.uint8);
	if (rc != 0) {
		printf("%s() ERROR, line %d\n", __func__, __LINE__);
		return rc;
	}
	return rc;
}


/*
 *  enable/disable network connection between 1G rj45 management port and cpu
 */
int siklu_cpu_netw_cntrl(int is_ena) {
	int rc = 0, count;
	T_CPLD_LOGIC_MODEM_LEDS_CTRL_REGS reset_reg;

	// 1. un-reset SOHO chip
	reset_reg.uint8 = siklu_cpld_read(R_CPLD_LOGIC_MODEM_LEDS_CTRL);
	reset_reg.s.cfg_switch_rst_n = 1;
	siklu_cpld_write(R_CPLD_LOGIC_MODEM_LEDS_CTRL, reset_reg.uint8);

	// 2. wait Xms
	udelay(50000); // 50msec?

	// 3. enable port0 (CPU) and port 5 (1G management)
	// 		disable all other ports
#define SOHO_FIRST_PORT			0
#define SOHO_LAST_PORT  		0xA
#define SOHO_HOST_CPU_PORT  	0x0
#define SOHO_MNGM_PORT  		0x5
#define SOHO_PORT_CONTROL_REG	4

	for (count = SOHO_FIRST_PORT; count <= SOHO_LAST_PORT; count++) {
		rc = siklu_88e639x_reg_write(count, SOHO_PORT_CONTROL_REG, 0x7C);
		if (rc != 0) {
			printf("%s() Disable SOHO port#%d FAIL\n", __func__, count);
			break;
		}
	}

	siklu_88e639x_reg_write(SOHO_HOST_CPU_PORT, SOHO_PORT_CONTROL_REG, 0x7F);
	siklu_88e639x_reg_write(SOHO_MNGM_PORT, SOHO_PORT_CONTROL_REG, 0x7F);

	if (siklu_get_board_type() != SKL_BOARD_TYPE_PCB195) {
		// follow setup required only for SOHO on PCB1213
		rc = siklu_88e639x_reg_write(SOHO_HOST_CPU_PORT, 0, 0xD05);
	}


	return rc;
}

/*
 *
 */
static int do_siklu_mngmnt_netw_ena_cntrl(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE, ret;
	int is_ena = simple_strtoul(argv[1], NULL, 10);

	ret = siklu_cpu_netw_cntrl(!!is_ena);
	if (ret==0)
		rc = CMD_RET_SUCCESS;
	return rc;
}



/*
 *
 */
static int do_siklu_soho_access(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;
	u8 page;
	u8 reg;
	u16 val;

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS0);
	switch (argc) {

	case 2: // display all registers in required page
		page = simple_strtoul(argv[1], NULL, 16);
		rc = soho_display_page(page);
		break;

	case 3: // display register in required page
		page = simple_strtoul(argv[1], NULL, 16);
		reg = simple_strtoul(argv[2], NULL, 16);
		rc = soho_display_register(page, reg);
		break;
	case 4: // set register value in required page
		page = simple_strtoul(argv[1], NULL, 16);
		reg = simple_strtoul(argv[2], NULL, 16);
		val = simple_strtoul(argv[3], NULL, 16);
		rc = soho_write_register(page, reg, val);
		break;
	case 1:  // display help
	default:
		printf("Usage:\n%s\n", cmdtp->usage);
		rc = CMD_RET_USAGE;
		break;
	}
	return rc;
}

/*
 *
 */
static int do_siklu_soho_ver_read(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;
	int ret;

#define SOHO_NXP_CPU_PORT 	0
#define SWITCH_IDENT_REG	3
	u16 val;

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS0);
	ret = siklu_88e639x_reg_read(SOHO_NXP_CPU_PORT, SWITCH_IDENT_REG, &val);
	if (ret == 0) {
		printf(" 0x%04X\n", val);
	} else {
		printf(" Read ERROR %d\n", ret);
	}
	return rc;
}


U_BOOT_CMD(sohoa, 5, 1, do_siklu_soho_access, "Read/write SOHO",
		" [port] [reg*] [val*] Read/write SOHO");

U_BOOT_CMD(sohov, 1, 1, do_siklu_soho_ver_read,
		"Read SOHO Product Number & revision",
		" Read SOHO Product Number & revision");


U_BOOT_CMD(snetw_ena, 5, 1, do_siklu_mngmnt_netw_ena_cntrl,
		"Network Management Enable control",
		" [is_ena 0/1] Network Management Enable control");

