/*
 * mx6ullevk_siklu_mdio.c
 *
 *  Created on: Dec 28, 2017
 *      Author: edwardk
 *
 *
 *      ENET1 allows access MDIO bus 1 and 2
 *      The file holds code for switch busses
 */

#include <common.h>
#include <linux/ctype.h>

#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx6ull_pins.h>

#include "siklu_def.h"
#include "siklu_api.h"


#define MDIO_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST | PAD_CTL_ODE)

# define MDC_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST ) // Siklu board pcb19x requires Open-Drain Disabled for MDC

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_SPEED_HIGH   |                                  \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST)


static const iomux_v3_cfg_t  fec_mdio_bus0_pads[] = {  // bus0 connected to SOHO
		MX6_PAD_GPIO1_IO07__ENET1_MDC | MUX_PAD_CTRL(MDC_PAD_CTRL),
		MX6_PAD_ENET2_RX_DATA1__ENET1_MDC | MUX_PAD_CTRL(MDC_PAD_CTRL),

		MX6_PAD_GPIO1_IO06__ENET1_MDIO | MUX_PAD_CTRL(MDIO_PAD_CTRL), // connect GPIO1_IO06 to ENET1 MDIO
		MX6_PAD_ENET2_RX_DATA0__ENET2_RDATA00 | MUX_PAD_CTRL(ENET_PAD_CTRL), // disconnect MDIO from ENET1
};


static const iomux_v3_cfg_t  fec_mdio_bus1_pads[] = {  // bus1 connected to 10G PHY
		MX6_PAD_GPIO1_IO07__ENET1_MDC | MUX_PAD_CTRL(MDC_PAD_CTRL),
		MX6_PAD_ENET2_RX_DATA1__ENET1_MDC | MUX_PAD_CTRL(MDC_PAD_CTRL),

		MX6_PAD_ENET2_RX_DATA0__ENET1_MDIO  | MUX_PAD_CTRL(MDIO_PAD_CTRL), // connect  ENET2_RX_DATA0 to ENET1 MDIO
		MX6_PAD_GPIO1_IO06__UART1_DTE_RTS| MUX_PAD_CTRL(NO_PAD_CTRL),      // disconnect GPIO1_IO06 from ENET1 MDIO

};

/*
 *
 */
int siklu_mdio_bus_connect(SIKLU_MDIO_BUS_E bus)
{
	int rc = 0;

	switch (bus)
	{
	case SIKLU_MDIO_BUS0:
		imx_iomux_v3_setup_multiple_pads(fec_mdio_bus0_pads,
								 ARRAY_SIZE(fec_mdio_bus0_pads));
		break;
	case SIKLU_MDIO_BUS1:
		imx_iomux_v3_setup_multiple_pads(fec_mdio_bus1_pads,
									 ARRAY_SIZE(fec_mdio_bus1_pads));
		break;
	default:
		rc = -1;
		break;
	}

	return rc;
}
