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

#include <miiphy.h>

#include "siklu_def.h"
#include "siklu_api.h"

#include <spi.h>


static const iomux_v3_cfg_t cpld_pads[] = { //
		MX6_PAD_CSI_DATA01__ECSPI2_SS0 | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_LCD_HSYNC__ECSPI2_SS1 | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_DATA00__ECSPI2_SCLK | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_DATA02__ECSPI2_MOSI | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_DATA03__ECSPI2_MISO | MUX_PAD_CTRL(NO_PAD_CTRL) //
//
		};


static const iomux_v3_cfg_t i2c1_pads[] = {
				MX6_PAD_CSI_MCLK__CSI_MCLK | MUX_PAD_CTRL(NO_PAD_CTRL),
				MX6_PAD_CSI_PIXCLK__CSI_PIXCLK | MUX_PAD_CTRL(NO_PAD_CTRL),
				MX6_PAD_CSI_MCLK__I2C1_SDA | MUX_PAD_CTRL(NO_PAD_CTRL),
				MX6_PAD_CSI_PIXCLK__I2C1_SCL | MUX_PAD_CTRL(NO_PAD_CTRL)
};

static const iomux_v3_cfg_t i2c2_pads[] = {
				MX6_PAD_CSI_HSYNC__CSI_HSYNC | MUX_PAD_CTRL(NO_PAD_CTRL),
				MX6_PAD_CSI_VSYNC__CSI_VSYNC | MUX_PAD_CTRL(NO_PAD_CTRL),
				MX6_PAD_CSI_VSYNC__I2C2_SDA | MUX_PAD_CTRL(NO_PAD_CTRL),
				MX6_PAD_CSI_HSYNC__I2C2_SCL | MUX_PAD_CTRL(NO_PAD_CTRL)
};


static void setup_iomux_siklu_cpld(void) {
	imx_iomux_v3_setup_multiple_pads(cpld_pads, ARRAY_SIZE(cpld_pads));
}

static void setup_iomux_siklu_i2c(void) {
	imx_iomux_v3_setup_multiple_pads(i2c1_pads, ARRAY_SIZE(i2c1_pads));
	imx_iomux_v3_setup_multiple_pads(i2c2_pads, ARRAY_SIZE(i2c2_pads));
}

int siklu_cpld_read(u8 reg, u8* data) {

	int rc = CMD_RET_FAILURE;

	const u32 bus = CONFIG_CPLD_DEFAULT_BUS;
	const u32 cs = CONFIG_CPLD_DEFAULT_CS;
	const u32 max_hz = CONFIG_CPLD_DEFAULT_SPEED;
	u32 spi_mode = CONFIG_CPLD_DEFAULT_MODE;
	struct spi_slave *spi;
	int ret;
	u8 tx_buf[10];
#define CPLD_READ_SEQ_LENGTH    4
#define SPI_READ_MEMORY_COMMAND 0x0B

	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	if (!spi) {
		printf("%s: Failed to set up slave\n", __func__);
		return rc;
	}

	ret = spi_claim_bus(spi);
	if (ret) {
		printf("%s: Failed to claim SPI bus: %d\n", __func__, ret);
		goto err_claim_bus;
	}

	memset(tx_buf,0,sizeof(tx_buf));

	tx_buf[0] = SPI_READ_MEMORY_COMMAND;
	tx_buf[1] = reg;

	ret = spi_xfer(spi, CPLD_READ_SEQ_LENGTH * 8, tx_buf, data,
	SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret < 0) {
		printf("%s: Failed XFER SPI: ret - %d\n", __func__, ret);
		goto err_claim_bus;
	}

	printf("\n RX buf: %2x %2x %2x (%2x)\n",data[0], data[1], data[2], data[3]);

	// last before exit
	spi_free_slave(spi);

	return CMD_RET_SUCCESS;
	err_claim_bus: spi_free_slave(spi);
	return CMD_RET_FAILURE;
}


int siklu_cpld_write(u8 reg, u8 data) {

	int rc = CMD_RET_FAILURE;

	const u32 bus = CONFIG_CPLD_DEFAULT_BUS;
	const u32 cs = CONFIG_CPLD_DEFAULT_CS;
	const u32 max_hz = CONFIG_CPLD_DEFAULT_SPEED;
	u32 spi_mode = CONFIG_CPLD_DEFAULT_MODE;
	struct spi_slave *spi;
	int ret;
	u8 tx_buf[10];
#define CPLD_WRITE_SEQ_LENGTH    3
#define SPI_WRITE_MEMORY_COMMAND 0x02

	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	if (!spi) {
		printf("%s: Failed to set up slave\n", __func__);
		return rc;
	}

	ret = spi_claim_bus(spi);
	if (ret) {
		printf("%s: Failed to claim SPI bus: %d\n", __func__, ret);
		goto err_claim_bus;
	}

	memset(tx_buf,0,sizeof(tx_buf));

	tx_buf[0] = SPI_WRITE_MEMORY_COMMAND;
	tx_buf[1] = reg;
	tx_buf[2] = data;

	ret = spi_xfer(spi, CPLD_WRITE_SEQ_LENGTH * 8, tx_buf, NULL,
	SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret < 0) {
		printf("%s: Failed XFER SPI: ret - %d\n", __func__, ret);
		goto err_claim_bus;
	}

	// last before exit
	spi_free_slave(spi);

	return CMD_RET_SUCCESS;
	err_claim_bus: spi_free_slave(spi);
	return CMD_RET_FAILURE;
}

/*
 *
 *
 */
int siklu_board_late_init(void) {
	int rc = 0;

	// TODO Via MDIO bus:
	// 		Enable SOHO Port#5 - output to network, configure 1G autoneg
	//		Enable SOHO Port#0 - connection to NXP SoC, configure strict 100FD
	siklu_mdio_bus_connect(SIKLU_MDIO_BUS0); // by default connect SOHO Switch
	return rc;
}

int siklu_board_init(void) {
	int rc = 0;
	setup_iomux_siklu_cpld();
	setup_iomux_siklu_i2c();

	// TODO In CPLD:
	// 	put to reset all unnecessary HW devices
	// 	release SOHO reset

	return rc;
}
