/*
 * mx6ullevk_siklu_clause45.c
 *
 *  Created on: Dec 26, 2017
 *      Author: edwardk
 *
 *      Access Marvell 10G 88x3310 PHY device
 *      Access TI
 *
 *
 */

#include <common.h>
#include <linux/ctype.h>
#include <command.h>
#include <version.h>

#include <miiphy.h>

#include "siklu_def.h"
#include "siklu_api.h"
#include "cpld_reg.h"

#include "../drivers/net/fec_mxc.h"


static const struct ethernet_regs *enet1_eth =
		(struct ethernet_regs *) 0x02188000;

static inline int _miiphy_write(const char *devname, unsigned char addr,
		unsigned char reg, unsigned short value) {
	miiphy_write(devname, addr, reg, value);
	return 0;
}

static inline int _miiphy_read(const char *devname, unsigned char addr,
		unsigned char reg, unsigned short *value) {
	miiphy_read(devname, addr, reg, value); //
	return 0;
}


void get_88x3310_phy_version(u32 *val_version)
{
	const char *devname;
	uint32_t dev_addr = 0, reg = 0;
	uint16_t val;


#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return;
	}

	dev_addr = 1;
	reg = 0x1;

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS1);

	// Phase #1 according to datasheet p93, pp3.11.1.1
	val = (0 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 13, val);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 14, reg);

	// Phase #2
	val = (1 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 13, val);
	if (_miiphy_read(devname, PHY_88x3310_DEV_ADDR, 14, &val) != 0) {
		printf("  ERROR read PHY\n");
		return;
	}

	*val_version = val;
//	printf("  [0x%04X]\n", *val_version);
}


void get_TLK10031_version(u32 *val_version)
{
	int ret;
	const char *devname;
	uint32_t dev_addr = 0, reg = 0;
	uint16_t addr_data;

#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return;
	}

	dev_addr = 1;
	reg = 0x3;

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS1);

	addr_data = reg; // preset reg address
	ret = fec_mdio_op_clause45(enet1_eth, CLAUSE45_OP_ADDR, TI10031_DEV_ADDR, dev_addr, &addr_data);
	if (ret != 0) {
		printf(" Error on line %d\n", __LINE__);
		return;
	}
	// on next operation addr_data will hold reg value
	ret = fec_mdio_op_clause45(enet1_eth, CLAUSE45_OP_READ, TI10031_DEV_ADDR, dev_addr, &addr_data);
	if (ret != 0) {
		printf(" Error on line %d\n", __LINE__);
		return;
	}

	*val_version = addr_data;
//	printf("  [0x%04X]\n", addr_data);
}

/*
 *
 */
static int do_siklu_read_88x3310_phy_aes(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_SUCCESS;
	const char *devname;
	uint32_t dev_addr = 0, reg = 0;
	uint16_t val;

	if (argc != 3) {
		printf("Wrong number arguments %d\n", argc);
		return CMD_RET_USAGE;
	}

#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return -1;
	}

	dev_addr = simple_strtoul(argv[1], NULL, 16);
	reg = 0xFFFF & simple_strtoul(argv[2], NULL, 16);

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS1);

	// Phase #1 according to datasheet p93, pp3.11.1.1
	val = (0 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_AES_88x3310_DEV_ADDR, 13, val);
	_miiphy_write(devname, PHY_AES_88x3310_DEV_ADDR, 14, reg);

	// Phase #2
	val = (1 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_AES_88x3310_DEV_ADDR, 13, val);
	if (_miiphy_read(devname, PHY_AES_88x3310_DEV_ADDR, 14, &val) != 0) {
		printf("  ERROR read PHY\n");
		return CMD_RET_FAILURE;
	}
	printf("  [0x%04X]\n", val);

	return rc;
}

/*
 *
 *
 */
static int do_siklu_write_88x3310_phy_aes(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_SUCCESS;
	const char *devname;
	uint32_t dev_addr = 0, reg_addr = 0;
	uint16_t reg_val = 0, temp;

	if (argc != 4) {
		printf("Wrong number arguments %d\n", argc);
		return CMD_RET_USAGE;
	}

#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return -1;
	}

	dev_addr = simple_strtoul(argv[1], NULL, 16);
	reg_addr = 0xFFFF & simple_strtoul(argv[2], NULL, 16);
	reg_val  = 0xFFFF & simple_strtoul(argv[3], NULL, 16);

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS1);

	// Phase #1 according to datasheet p93, pp3.11.1.1
	temp = (0 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_AES_88x3310_DEV_ADDR, 13, temp);
	_miiphy_write(devname, PHY_AES_88x3310_DEV_ADDR, 14, reg_addr);

	// Phase #2
	 temp = (1 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_AES_88x3310_DEV_ADDR, 13, temp);
	_miiphy_write(devname, PHY_AES_88x3310_DEV_ADDR, 14, reg_val);
	return rc;
}

static int do_siklu_88x3310_phy_aes_enable_control(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = 0;
	T_CPLD_LOGIC_MODEM_LEDS_CTRL_REGS reset_reg;

	int is_ena = simple_strtoul(argv[1], NULL, 10);

	reset_reg.uint8 = siklu_cpld_read(R_CPLD_LOGIC_MODEM_LEDS_CTRL);

	reset_reg.s.cfg_aes_phy_rst_n = (!!is_ena);
	rc = siklu_cpld_write(R_CPLD_LOGIC_MODEM_LEDS_CTRL, reset_reg.uint8);
	if (rc != 0) {
		printf("%s() ERROR, line %d\n", __func__, __LINE__);
		return rc;
	}
	return rc;
}

/*
 *
 */
static int do_siklu_read_88x3310_phy(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;
	const char *devname;
	uint32_t dev_addr = 0, reg = 0;
	uint16_t val;

	if (argc != 3) {
		printf("Wrong number arguments %d\n", argc);
		return CMD_RET_USAGE;
	}

#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return -1;
	}

	dev_addr = simple_strtoul(argv[1], NULL, 16);
	reg = 0xFFFF & simple_strtoul(argv[2], NULL, 16);

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS1);

	// Phase #1 according to datasheet p93, pp3.11.1.1
	val = (0 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 13, val);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 14, reg);

	// Phase #2
	val = (1 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 13, val);
	if (_miiphy_read(devname, PHY_88x3310_DEV_ADDR, 14, &val) != 0) {
		printf("  ERROR read PHY\n");
		return CMD_RET_FAILURE;
	}
	printf("  [0x%04X]\n", val);

	return rc;
}

/*
 *
 *
 */
static int do_siklu_write_88x3310_phy(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;
	const char *devname;
	uint32_t dev_addr = 0, reg_addr = 0;
	uint16_t reg_val = 0, temp;

	if (argc != 4) {
		printf("Wrong number arguments %d\n", argc);
		return CMD_RET_USAGE;
	}

#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return -1;
	}

	dev_addr = simple_strtoul(argv[1], NULL, 16);
	reg_addr = 0xFFFF & simple_strtoul(argv[2], NULL, 16);
	reg_val  = 0xFFFF & simple_strtoul(argv[3], NULL, 16);

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS1);

	// Phase #1 according to datasheet p93, pp3.11.1.1
	temp = (0 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 13, temp);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 14, reg_addr);

	// Phase #2
	 temp = (1 << 14) | (dev_addr & 0x1f);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 13, temp);
	_miiphy_write(devname, PHY_88x3310_DEV_ADDR, 14, reg_val);
	return rc;
}

/*
 *
 */
static int do_siklu_read_TLK10031_reg(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int  ret;

	const char *devname;
	uint32_t dev_addr = 0, reg = 0;
	uint16_t addr_data;

	if (argc != 3) {
		printf("Wrong number arguments %d\n", argc);
		return CMD_RET_USAGE;
	}

#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return CMD_RET_FAILURE;
	}

	dev_addr = simple_strtoul(argv[1], NULL, 16);
	reg = 0xFFFF & simple_strtoul(argv[2], NULL, 16);

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS1);

	addr_data = reg; // preset reg address
	ret = fec_mdio_op_clause45(enet1_eth, CLAUSE45_OP_ADDR, TI10031_DEV_ADDR, dev_addr, &addr_data);
	if (ret != 0) {
		printf(" Error on line %d\n", __LINE__);
		return CMD_RET_FAILURE;
	}
	// on next operation addr_data will hold reg value
	ret = fec_mdio_op_clause45(enet1_eth, CLAUSE45_OP_READ, TI10031_DEV_ADDR, dev_addr, &addr_data);
	if (ret != 0) {
		printf(" Error on line %d\n", __LINE__);
		return CMD_RET_FAILURE;
	}

	printf("  [0x%04X]\n", addr_data);
	return CMD_RET_SUCCESS;
}

/*
 *
 */
static int do_siklu_write_TLK10031_reg(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int ret;

	const char *devname;
	uint32_t dev_addr = 0, reg_addr = 0;
	uint16_t addr_data, reg_val;

	if (argc != 4) {
		printf("Wrong number arguments %d\n", argc);
		return CMD_RET_USAGE;
	}

#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return CMD_RET_FAILURE;
	}

	dev_addr = simple_strtoul(argv[1], NULL, 16);
	reg_addr = 0xFFFF & simple_strtoul(argv[2], NULL, 16);
	reg_val = 0xFFFF & simple_strtoul(argv[3], NULL, 16);

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS1);

	addr_data = reg_addr; // preset reg_addr address
	ret = fec_mdio_op_clause45(enet1_eth, CLAUSE45_OP_ADDR, TI10031_DEV_ADDR, dev_addr, &addr_data);
	if (ret != 0) {
		printf(" Error on line %d\n", __LINE__);
		return CMD_RET_FAILURE;
	}
	addr_data = reg_val;// preset reg_val value
	ret = fec_mdio_op_clause45(enet1_eth, CLAUSE45_OP_WRITE, TI10031_DEV_ADDR, dev_addr, &addr_data);
	if (ret != 0) {
		printf(" Error on line %d\n", __LINE__);
		return CMD_RET_FAILURE;
	}
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(phy_aesr, 5, 1, do_siklu_read_88x3310_phy_aes, "Read 88x3310 AES PHY",
		" [dev_addr] [reg]");

U_BOOT_CMD(phy_aesw, 5, 1, do_siklu_write_88x3310_phy_aes, "Write 88x3310 AES PHY",
		" [dev_addr] [reg] [val]");

U_BOOT_CMD(phy_aes_ena, 5, 1, do_siklu_88x3310_phy_aes_enable_control, "PHY AES 88x3310P Enable control",
		" [is_ena 0/1] PHY AES 88x3310P Enable control");

U_BOOT_CMD(phy10r, 5, 1, do_siklu_read_88x3310_phy, "Read 88x3310 10G PHY",
		" [dev_addr] [reg]");

U_BOOT_CMD(phy10w, 5, 1, do_siklu_write_88x3310_phy, "Write 88x3310 10G PHY",
		" [dev_addr] [reg] [val]");

U_BOOT_CMD(tlkr, 5, 1, do_siklu_read_TLK10031_reg, "Read TLK10031 Transceiver",
		" [dev_addr] [reg]");

U_BOOT_CMD(tlkw, 5, 1, do_siklu_write_TLK10031_reg,
		"Write TLK10031 Transceiver", " [dev_addr] [reg] [val]");

