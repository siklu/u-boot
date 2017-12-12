/*
 * mx6ullevk_siklu_hw.c
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

#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>

#include "siklu_def.h"
#include "siklu_api.h"

#define MDIO_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST | PAD_CTL_ODE)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_SPEED_HIGH   |                                  \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST)

#define ENET_CLK_PAD_CTRL  (PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

static const iomux_v3_cfg_t cpld_pads[] = { //
		MX6_PAD_CSI_DATA01__ECSPI2_SS0 | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_LCD_HSYNC__ECSPI2_SS1 | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_DATA00__ECSPI2_SCLK | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_DATA02__ECSPI2_MOSI | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_DATA03__ECSPI2_MISO | MUX_PAD_CTRL(NO_PAD_CTRL) //
//
		};

static void setup_iomux_siklu_cpld(void)
{
	imx_iomux_v3_setup_multiple_pads(cpld_pads, ARRAY_SIZE(cpld_pads));
}


 int siklu_cpld_read(u8 reg, u8* data)
{
	int rc = 0;
	return rc;
}
 int siklu_cpld_write(u8 reg, u8 data)
{
	int rc = 0;
	return rc;
}





int siklu_board_init(void) {
	int rc = 0;
	setup_iomux_siklu_cpld();

	return rc;
}
