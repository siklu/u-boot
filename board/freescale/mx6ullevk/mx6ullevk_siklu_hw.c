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

#define GPIO_ECSPI1_SS_MANUAL_CONTROL 1 // in this mode ECSPI1_SS0 andECSPI1_SS1  controlled via GPIO, not by controller!

#ifdef GPIO_ECSPI1_SS_MANUAL_CONTROL
# define GPIO_ECSPI1_SS0 IMX_GPIO_NR(4, 26)
# define GPIO_ECSPI1_SS1 IMX_GPIO_NR(3, 10)
# define GPIO_ECSPI1_SS2 IMX_GPIO_NR(3, 11)
# define GPIO_ECSPI1_SS3 IMX_GPIO_NR(3, 12)

#endif // GPIO_ECSPI1_SS_MANUAL_CONTROL

static const iomux_v3_cfg_t rfic_pads[] = { //
		/*  */
#ifndef GPIO_ECSPI1_SS_MANUAL_CONTROL
				MX6_PAD_CSI_DATA05__ECSPI1_SS0 | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_LCD_DATA05__ECSPI1_SS1 | MUX_PAD_CTRL(NO_PAD_CTRL),//
				MX6_PAD_LCD_DATA06__ECSPI1_SS2 | MUX_PAD_CTRL(NO_PAD_CTRL),//
				MX6_PAD_LCD_DATA07__ECSPI1_SS3 | MUX_PAD_CTRL(NO_PAD_CTRL),//
#else
				MX6_PAD_CSI_DATA05__GPIO4_IO26 | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_LCD_DATA05__GPIO3_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_LCD_DATA06__GPIO3_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_LCD_DATA07__GPIO3_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL), //
#endif //
				MX6_PAD_CSI_DATA04__ECSPI1_SCLK | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_DATA06__ECSPI1_MOSI | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_DATA07__ECSPI1_MISO | MUX_PAD_CTRL(NO_PAD_CTRL) //
//
		};

static const iomux_v3_cfg_t i2c1_pads[] = { //
		MX6_PAD_CSI_MCLK__CSI_MCLK | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_PIXCLK__CSI_PIXCLK | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_MCLK__I2C1_SDA | MUX_PAD_CTRL(NO_PAD_CTRL), //
				MX6_PAD_CSI_PIXCLK__I2C1_SCL | MUX_PAD_CTRL(NO_PAD_CTRL) //
		};

static const iomux_v3_cfg_t i2c2_pads[] = { MX6_PAD_CSI_HSYNC__CSI_HSYNC
		| MUX_PAD_CTRL(NO_PAD_CTRL), MX6_PAD_CSI_VSYNC__CSI_VSYNC
		| MUX_PAD_CTRL(NO_PAD_CTRL), MX6_PAD_CSI_VSYNC__I2C2_SDA
		| MUX_PAD_CTRL(NO_PAD_CTRL), MX6_PAD_CSI_HSYNC__I2C2_SCL
		| MUX_PAD_CTRL(NO_PAD_CTRL) };

static void setup_iomux_siklu_cpld(void) {
	imx_iomux_v3_setup_multiple_pads(cpld_pads, ARRAY_SIZE(cpld_pads));
}

static void setup_iomux_siklu_i2c(void) {
	imx_iomux_v3_setup_multiple_pads(i2c1_pads, ARRAY_SIZE(i2c1_pads));
	imx_iomux_v3_setup_multiple_pads(i2c2_pads, ARRAY_SIZE(i2c2_pads));
}

int board_spi_cs_gpio(unsigned bus, unsigned cs) {
	int rc = -1;

	switch (bus) {
	case 0:   // RF not functional in uboot
#ifdef  _GPIO_ECSPI1_SS_MANUAL_CONTROL // code below disabled. MXC_SPI doesn't control CS output, we use this manually
	{
		if (cs == 0)
		rc = IMX_GPIO_NR(4, 26);
		else if (cs == 1)
		rc = IMX_GPIO_NR(3, 10);
	}
#endif //
		break;
	case 1:  // CPLD
		break;
	case 2: // serial-NOR
	{
		if (cs == 0) {
			rc = IMX_GPIO_NR(1, 20);
		}
	}
		break;
	default:
		break;

	}

	return rc;
}

/*
 * init access only once
 */
static int siklu_rfic_module_access_init(void) {
	int rc = 0;
	static int is_init = 0;

	if (is_init)
		return rc;

	imx_iomux_v3_setup_multiple_pads(rfic_pads, ARRAY_SIZE(rfic_pads));

#ifdef 	GPIO_ECSPI1_SS_MANUAL_CONTROL
	gpio_direction_output(GPIO_ECSPI1_SS0, 1); // init all RFIC module CS (total 4) by HIGH
	gpio_direction_output(GPIO_ECSPI1_SS1, 1);
	gpio_direction_output(GPIO_ECSPI1_SS2, 1);
	gpio_direction_output(GPIO_ECSPI1_SS3, 1);
#endif //

	is_init = 1;
	return rc;
}

static int siklu_rfic_1byte_xfer(int spi_mode, u32 cs, unsigned long flags,
		u8 dout, u8* din) {
	int rc = 0, ret;
	u32 bus = CONFIG_RFIC_DEFAULT_BUS;
	u32 max_hz = CONFIG_RFIC_DEFAULT_SPEED;
	struct spi_slave *spi;
	u8 tx_buf[5];
	u8 rx_buf[5];

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

	memset(tx_buf, 0, sizeof(tx_buf));
	memset(rx_buf, 0, sizeof(rx_buf));
	tx_buf[0] = dout;

	ret = spi_xfer(spi, 1 * 8, tx_buf, rx_buf, flags);
	if (ret < 0) {
		printf("%s: Failed XFER SPI: ret - %d\n", __func__, ret);
		goto err_claim_bus;
	}

	*din = rx_buf[0];
	spi_free_slave(spi);

	return rc;
	err_claim_bus: //
	spi_free_slave(spi);
	return -1;

}

/*
 *
 */
int siklu_rfic_module_read(MODULE_RFIC_E module, u8 reg, u8* data) {
	int rc = 0;
	u32 cs;
	u32 spi_mode;
	u8 din;
	u32 gpio_cs;

	siklu_rfic_module_access_init();
	if (module == MODULE_RFIC_70) {
		cs = CONFIG_RFIC70_DEFAULT_CS;
		gpio_cs = GPIO_ECSPI1_SS0;
	} else {
		cs = CONFIG_RFIC80_DEFAULT_CS;
		gpio_cs = GPIO_ECSPI1_SS1;
	}

	gpio_direction_output(gpio_cs, 0);

	spi_mode = 0; // write data to RFIC in mode#0
	rc = siklu_rfic_1byte_xfer(spi_mode, cs, SPI_XFER_BEGIN, reg | 0x80, &din);
	if (rc != 0) {
		printf("%s() error on line %d\n", __func__, __LINE__);
		return rc;
	}
	spi_mode = 1; // read data from RFIC in mode#1
	rc = siklu_rfic_1byte_xfer(spi_mode, cs, SPI_XFER_END, 0xFF, data);
	if (rc != 0) {
		printf("%s() error on line %d\n", __func__, __LINE__);
		return rc;
	}
	gpio_direction_output(gpio_cs, 1);
	return 0;

}

int siklu_rfic_module_write(MODULE_RFIC_E module, u8 reg, u8 data) {
	int ret = -1;
	u32 cs;
	u32 spi_mode = 0;
	u8 __attribute__((unused)) din;
	u32 gpio_cs;
	u32 bus = CONFIG_RFIC_DEFAULT_BUS;
	u8 tx_buf[10];
	const u32 max_hz = CONFIG_CPLD_DEFAULT_SPEED;
	struct spi_slave *spi;

	siklu_rfic_module_access_init();
	if (module == MODULE_RFIC_70) {
		cs = CONFIG_RFIC70_DEFAULT_CS;
		gpio_cs = GPIO_ECSPI1_SS0;
	} else {
		cs = CONFIG_RFIC80_DEFAULT_CS;
		gpio_cs = GPIO_ECSPI1_SS1;
	}

	gpio_direction_output(gpio_cs, 0);

	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	if (!spi) {
		printf("%s: Failed to set up slave\n", __func__);
		return ret;
	}

	ret = spi_claim_bus(spi);
	if (ret) {
		printf("%s: Failed to claim SPI bus: %d\n", __func__, ret);
		goto err_claim_bus;
	}

	memset(tx_buf, 0, sizeof(tx_buf));

	tx_buf[0] = reg & 0x7F;
	tx_buf[1] = data;

	ret = spi_xfer(spi, 2 * 8, tx_buf, NULL, SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret < 0) {
		printf("%s: Failed XFER SPI: ret - %d\n", __func__, ret);
		goto err_claim_bus;
	}

	// last before exit
	gpio_direction_output(gpio_cs, 1);
	spi_free_slave(spi);

	return 0;
	err_claim_bus: //
	gpio_direction_output(gpio_cs, 1);
	spi_free_slave(spi);
	return ret;

}

/*
 *
 */
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

	memset(tx_buf, 0, sizeof(tx_buf));

	tx_buf[0] = SPI_READ_MEMORY_COMMAND;
	tx_buf[1] = reg;

	ret = spi_xfer(spi, CPLD_READ_SEQ_LENGTH * 8, tx_buf, data,
	SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret < 0) {
		printf("%s: Failed XFER SPI: ret - %d\n", __func__, ret);
		goto err_claim_bus;
	}

	printf("\n RX buf: %2x %2x %2x (%2x)\n", data[0], data[1], data[2],
			data[3]);

	// last before exit
	spi_free_slave(spi);

	return CMD_RET_SUCCESS;
	err_claim_bus: //
	spi_free_slave(spi);
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

	memset(tx_buf, 0, sizeof(tx_buf));

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
	siklu_rfic_module_access_init();

	// TODO In CPLD:
	// 	put to reset all unnecessary HW devices
	// 	release SOHO reset

	return rc;
}
