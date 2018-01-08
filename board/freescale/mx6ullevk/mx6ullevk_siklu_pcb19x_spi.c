/*
 * mx6ullevk_siklu_pcb19x_spi.c
 *
 *  Created on: Dec 10, 2017
 *      Author: edwardk
 */

#include <common.h>
#include <command.h>
#include <version.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/compiler.h>

#include "siklu_def.h"
#include "siklu_api.h"
#include <spi.h>

/*
 *
 */
static int do_siklu_snor_mid_read(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;

	const u32 bus = CONFIG_SF_DEFAULT_BUS;
	const u32 cs = CONFIG_SF_DEFAULT_CS;
	const u32 max_hz = CONFIG_SF_DEFAULT_SPEED;
	u32 spi_mode = CONFIG_SF_DEFAULT_MODE;
	struct spi_slave *spi;
	int ret;
#define MID_SEQ_LENGTH 4
	u8 tx_buf[10];
	u8 rx_buf[10];

	if (argc > 1) {
		spi_mode = simple_strtoul(argv[1], NULL, 10);
	}

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
	memset(rx_buf, 0x00, sizeof(rx_buf));

	tx_buf[0] = 0x9F; // revision ID command

	ret = spi_xfer(spi, MID_SEQ_LENGTH * 8, tx_buf, rx_buf,
	SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret < 0) {
		printf("%s: Failed XFER SPI: ret - %d\n", __func__, ret);
		goto err_claim_bus;
	}

	printf("\n RX buf: %2x %2x %2x %2x\n", rx_buf[0], rx_buf[1], rx_buf[2],
			rx_buf[3]);

	// last before exit
	spi_free_slave(spi);

	return CMD_RET_SUCCESS;
	err_claim_bus: spi_free_slave(spi);
	return CMD_RET_FAILURE;

}

/*
 *
 */
static int do_siklu_snor_jedec_read(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;

	const u32 bus = CONFIG_SF_DEFAULT_BUS;
	const u32 cs = CONFIG_SF_DEFAULT_CS;
	const u32 max_hz = CONFIG_SF_DEFAULT_SPEED;
	u32 spi_mode = CONFIG_SF_DEFAULT_MODE;
	struct spi_slave *spi;
	int ret;
#define JEDEC_SEQ_LENGTH 6
	u8 tx_buf[10];
	u8 rx_buf[10];

	if (argc > 1) {
		spi_mode = simple_strtoul(argv[1], NULL, 10);
	}

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

	tx_buf[0] = 0x90; // revision ID command

	ret = spi_xfer(spi, JEDEC_SEQ_LENGTH * 8, tx_buf, rx_buf,
	SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret < 0) {
		printf("%s: Failed XFER SPI: ret - %d\n", __func__, ret);
		goto err_claim_bus;
	}

	printf("\n RX buf: %2x %2x %2x %2x (%2x %2x)\n", rx_buf[0], rx_buf[1],
			rx_buf[2], rx_buf[3], rx_buf[4], rx_buf[5]);

	// last before exit
	spi_free_slave(spi);

	return CMD_RET_SUCCESS;
	err_claim_bus: spi_free_slave(spi);
	return CMD_RET_FAILURE;

}



/*
 *
 */
static int do_siklu_rfic_read(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE, ret;
	int module = 0;
	MODULE_RFIC_E _mod;
	u8 val = 0, reg_addr = 0x7F;

	if (argc == 3) {
		module = simple_strtoul(argv[1], NULL, 10);
		reg_addr = simple_strtoul(argv[2], NULL, 16);
	} else {
		printf(" Not enough arguments - %d\n", argc);
		return CMD_RET_USAGE;
	}

	if (module == 70)
		_mod = MODULE_RFIC_70;
	else if (module == 80)
		_mod = MODULE_RFIC_80;
	else {
		printf(" Wrong module - %d\n", module);
		return CMD_RET_USAGE;
	}

	ret = siklu_rfic_module_read(_mod, reg_addr, &val);
	if (ret == 0)
		printf(" RFIC%d, reg 0x%2x, value 0x%x\n", module, reg_addr, val);
	else {
		printf(" Read fail\n");
		rc = CMD_RET_FAILURE;
	}

	return rc;
}

static int do_siklu_cpld_version_read(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;

	const u32 bus = CONFIG_CPLD_DEFAULT_BUS;
	const u32 cs = CONFIG_CPLD_DEFAULT_CS;
	const u32 max_hz = CONFIG_CPLD_DEFAULT_SPEED;
	u32 spi_mode = CONFIG_CPLD_DEFAULT_MODE;

	struct spi_slave *spi;
	int ret;
#define CPLD_VERSION_SEQ_LENGTH 3
	u8 tx_buf[5];
	u8 rx_buf[5];

	if (argc > 1) {
		spi_mode = simple_strtoul(argv[1], NULL, 10);
	}

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
	memset(rx_buf, 0x55, sizeof(rx_buf));

	tx_buf[0] = 0x9F; // revision ID command

	ret = spi_xfer(spi, CPLD_VERSION_SEQ_LENGTH * 8, tx_buf, rx_buf,
	SPI_XFER_BEGIN | SPI_XFER_END);
	if (ret < 0) {
		printf("%s: Failed XFER SPI: ret - %d\n", __func__, ret);
		goto err_claim_bus;
	}

	printf("\n RX buf: %2x %2x (%2x) %2x\n", rx_buf[0], rx_buf[1], rx_buf[2],
			rx_buf[3]);

	// last before exit
	spi_free_slave(spi);

	return CMD_RET_SUCCESS;
	err_claim_bus: spi_free_slave(spi);
	return CMD_RET_FAILURE;

}

static int do_siklu_cpld_read(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;

	u8 addr;
	u8 rx_buf[10];

	if (argc > 1) {
		addr = simple_strtoul(argv[1], NULL, 16);
	} else {
		printf("%s: Not enough arguments\n", __func__);
		return rc;
	}

	memset(rx_buf, 0, sizeof(rx_buf));

	return siklu_cpld_read(addr, rx_buf);

}

static int do_siklu_cpld_write(cmd_tbl_t * cmdtp, int flag, int argc,
		char * const argv[]) {
	int rc = CMD_RET_FAILURE;

	u8 addr, data;

	if (argc < 3) {
		printf("%s: Not enough arguments\n", __func__);
		return rc;
	}

	addr = simple_strtoul(argv[1], NULL, 16);
	data = simple_strtoul(argv[2], NULL, 16);

	return siklu_cpld_write(addr, data);
}

U_BOOT_CMD(scpldv, 5, 0, do_siklu_cpld_version_read,
		"Read Siklu CPLD version register",
		" [spi_mode 0..3*] Read Siklu CPLD version register");

U_BOOT_CMD(scpldr, 5, 0, do_siklu_cpld_read, "Read Siklu CPLD register",
		"[cpld read addr]");

U_BOOT_CMD(scpldw, 5, 0, do_siklu_cpld_write, "Write Siklu CPLD register",
		"[cpld write addr] [val]");

U_BOOT_CMD(snor_jdec, 5, 0, do_siklu_snor_jedec_read,
		"Read serial-NOR JEDEC data",
		" [spi_mode 0..3*] Read serial-NOR JEDEC data");

U_BOOT_CMD(snor_mid, 5, 0, do_siklu_snor_mid_read,
		"Read serial-NOR Manufacture ID", " Read serial-NOR Manufacture ID");

U_BOOT_CMD(srficr, 5, 0, do_siklu_rfic_read, "Read RFIC register",
		"[module: 70/80] [reg]");
