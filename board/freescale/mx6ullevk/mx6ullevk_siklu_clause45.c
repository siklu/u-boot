/*
 * mx6ullevk_siklu_clause45.c
 *
 *  Created on: Dec 26, 2017
 *      Author: edwardk
 *
 *      Access Marvell 10G 88x3310 PHY device
 *
 */

#include <common.h>
#include <linux/ctype.h>
#include <command.h>
#include <version.h>

/*
 #include <asm/arch/iomux.h>
 #include <asm/arch/imx-regs.h>
 #include <asm/arch/crm_regs.h>
 #include <asm/arch/mx6-pins.h>
 #include <asm/arch/sys_proto.h>
 #include <asm/gpio.h>
 #include <asm/mach-imx/iomux-v3.h>
 */
#include <miiphy.h>

#include "siklu_def.h"
#include "siklu_api.h"

#define PHY_88x3310_DEV_ADDR 2


static inline int _miiphy_write(const char *devname, unsigned char addr, unsigned char reg,
		  unsigned short value)
{
	miiphy_write(devname, addr, reg, value);
	return 0;
}


static inline int _miiphy_read(const char *devname, unsigned char addr, unsigned char reg,
		 unsigned short *value)
{
	miiphy_read(devname, addr, reg, value); //
	return 0;
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

	if (0) // for test only
	{
		if (_miiphy_read(devname, PHY_88x3310_DEV_ADDR, 22, &val) != 0) {
			printf("  ERROR read PHY, line %d\n", __LINE__);
			return CMD_RET_FAILURE;
		}
	}

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

	return rc;
}

U_BOOT_CMD(phy10r, 5, 1, do_siklu_read_88x3310_phy, "Read 88x3310 10G PHY",
		" [dev_addr] [reg]");

U_BOOT_CMD(phy10w, 5, 1, do_siklu_write_88x3310_phy, "Write 88x3310 10G PHY",
		" [dev_addr] [reg] [val]");
