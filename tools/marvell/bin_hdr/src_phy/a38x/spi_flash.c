/*
 * SPI flash interface
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 *
 * Licensed under the GPL-2 or later.
 */

#include <bootstrap_os.h>
#include <mvUart.h>
#include <common.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"
void __udelay (unsigned long usec);
#define mvOsUDelay         __udelay

static void spi_flash_addr(u32 addr, u8 *cmd, u8 addr_cycles)
{
	/* cmd[0] is actual command */
	switch  (addr_cycles) {
		case 4:
			cmd[1] = addr >> 24;
			cmd[2] = addr >> 16;
			cmd[3] = addr >> 8;
			cmd[4] = addr;
			break;
		case 3:
		default:
			cmd[1] = addr >> 16;
			cmd[2] = addr >> 8;
			cmd[3] = addr >> 0;
			break;
	}
}

extern struct spi_slave slave_global;

static int spi_flash_read_write(struct spi_slave *spi,
				const u8 *cmd, size_t cmd_len,
				const u8 *data_out, u8 *data_in,
				size_t data_len)
{
    //DEBUG_INIT_S("Entering spi_flash_read_write\n");
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;

	if (data_len == 0)
		flags |= SPI_XFER_END;

	ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags);
	if (ret) {
        DEBUG_INIT_S("SF: Failed to send command\n");
	} else if (data_len != 0) {
		ret = spi_xfer(spi, data_len * 8, data_out, data_in, SPI_XFER_END);
		if (ret)
            DEBUG_INIT_S("SF: Failed to transfer data\n");
	}

	return ret;
}

int spi_flash_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len)
{
    //DEBUG_INIT_S("Entering spi_flash_cmd\n");
	return spi_flash_cmd_read(spi, &cmd, 1, response, len);
}

int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	//printf("SF: cmd:0%x cmd_len:%d %s %s %d\n", *cmd, cmd_len, __FILE__, __FUNCTION__, __LINE__);
    //DEBUG_INIT_S("Entering spi_flash_cmd_read\n");
	return spi_flash_read_write(spi, cmd, cmd_len, NULL, data, data_len);
}

int spi_flash_cmd_write(struct spi_slave *spi, const u8 *cmd, size_t cmd_len,
		const void *data, size_t data_len)
{
	return spi_flash_read_write(spi, cmd, cmd_len, data, NULL, data_len);
}

int spi_flash_cmd_write_multi(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	unsigned long page_addr, byte_addr, page_size;
	size_t chunk_len, actual;
	int ret;
	u8 cmd[5];

	page_size = flash->page_size;
	page_addr = offset / page_size;
	byte_addr = offset % page_size;

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		DEBUG_INIT_S("SF: unable to claim SPI bus\n");
		return ret;
	}

	cmd[0] = CMD_PAGE_PROGRAM;
	for (actual = 0; actual < len; actual += chunk_len) {
		chunk_len = min(len - actual, page_size - byte_addr);

		switch  (flash->addr_cycles) {
			case 4:
				cmd[1] = page_addr >> 16;
				cmd[2] = page_addr >> 8;
				cmd[3] = page_addr;
				cmd[4] = byte_addr;
				break;
			case 3:
			default:
				cmd[1] = page_addr >> 8;
				cmd[2] = page_addr;
				cmd[3] = byte_addr;
				break;
		}

		ret = spi_flash_cmd_write_enable(flash);
		if (ret < 0) {
			DEBUG_INIT_S("SF: enabling write failed\n");
			break;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, flash->addr_cycles+1,
					  buf + actual, chunk_len);
		if (ret < 0) {
			DEBUG_INIT_S("SF: write failed\n");
			break;
		}

		ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret)
			break;

		page_addr++;
		byte_addr = 0;
#ifdef PRINT_DOT
	{
		print1block += chunk_len;
		if (print1block > DOT_WRITE_BLOCK) {
			printf(".");
			print1block -= DOT_WRITE_BLOCK;
			countDot++;
			if (countDot > 60) {
				printf("\n");
				countDot=0;
			}
		}
	}
#endif
	}

	DEBUG_INIT_S("SF: program bytes\n");

	spi_release_bus(flash->spi);
	return ret;
}

int spi_flash_read_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	struct spi_slave *spi = flash->spi;
	int ret;

	spi_claim_bus(spi);
	ret = spi_flash_cmd_read(spi, cmd, cmd_len, data, data_len);
	spi_release_bus(spi);

	return ret;
}

int spi_flash_cmd_read_fast(struct spi_flash *flash, u32 offset,
		size_t len, void *data)
{
	u8 cmd[6];
	cmd[0] = CMD_READ_ARRAY_FAST;
	spi_flash_addr(offset, cmd, flash->addr_cycles);

	cmd[flash->addr_cycles+1] = 0x00;

	return spi_flash_read_common(flash, cmd, flash->addr_cycles+2, data, len);
}

int spi_flash_cmd_poll_bit(struct spi_flash *flash, unsigned long timeout,
			   u8 cmd, u8 poll_bit)
{
	struct spi_slave *spi = flash->spi;
	int ret;
	u8 status;

	ret = spi_xfer(spi, 8, &cmd, NULL, SPI_XFER_BEGIN);
	if (ret) {
		DEBUG_INIT_S("SF: Failed to send command\n");
		return ret;
	}

    MV_U32 secondsCounter = 0;
    u8 maxIterations = 60;
	do {
        secondsCounter++;
	    //DEBUG_INIT_S("SF: trying spi_xfer...\n");
	    //DEBUG_INIT_S("secondsCounter: ");
	    //DEBUG_INIT_D(secondsCounter, 8);
	    //DEBUG_INIT_S("\n");
		ret = spi_xfer(spi, 8, NULL, &status, 0);
		if (ret)
			return -1;

		if ((status & poll_bit) == 0)
			break;

		mvOsUDelay(1000); // 1 ms
	} while (secondsCounter < maxIterations);

	spi_xfer(spi, 0, NULL, NULL, SPI_XFER_END);

	if ((status & poll_bit) == 0)
		return 0;

	/* Timed out */
	DEBUG_INIT_S("SF: time out!\n");
	return -1;
}

int spi_flash_cmd_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	return spi_flash_cmd_poll_bit(flash, timeout,
		CMD_READ_STATUS, STATUS_WIP);
}

int spi_flash_cmd_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	u32 start, end, erase_size;
	int ret;
	u8 cmd[5];
#ifdef PRINT_DOT
	int dor_erase_len = DOT_ERASE_BLOCK;
#endif
	erase_size = flash->sector_size;
	if (offset % erase_size || len % erase_size) {
		DEBUG_INIT_S("SF: Erase offset/length not multiple of erase size\n");
		return -1;
	}

	ret = spi_claim_bus(flash->spi);
	if (ret) {
		DEBUG_INIT_S("SF: Unable to claim SPI bus\n");
		return ret;
	}

	if (erase_size == 4096)
		cmd[0] = CMD_ERASE_4K;
	else
		cmd[0] = CMD_ERASE_64K;
	start = offset;
	end = start + len;

	while (offset < end) {
		spi_flash_addr(offset, cmd, flash->addr_cycles);
		offset += erase_size;

		DEBUG_INIT_S("SF: erase\n");

		ret = spi_flash_cmd_write_enable(flash);
		if (ret)
			goto out;

		ret = spi_flash_cmd_write(flash->spi, cmd, flash->addr_cycles+1, NULL, 0);
		if (ret)
			goto out;

		ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		if (ret)
			goto out;
#ifdef PRINT_DOT
	{
		dor_erase_len += erase_size;
		if (dor_erase_len > DOT_ERASE_BLOCK) {
			DEBUG_INIT_S(".");
			dor_erase_len -= DOT_ERASE_BLOCK;
			countDot++;
			if (countDot > 60) {
				DEBUG_INIT_S("\n");
				countDot=0;
			}
		}
	}
#endif
	}

	//DEBUG_INIT_S("SF: Successfully erased ");
    //DEBUG_INIT_D(len,4);
	//DEBUG_INIT_S("bytes\n");

 out:
	spi_release_bus(flash->spi);
	return ret;
}

int spi_flash_cmd_write_status(struct spi_flash *flash, u8 sr)
{
	u8 cmd;
	int ret;

	ret = spi_flash_cmd_write_enable(flash);
	if (ret < 0) {
		DEBUG_INIT_S("SF: enabling write failed\n");
		return ret;
	}

	cmd = CMD_WRITE_STATUS;
	ret = spi_flash_cmd_write(flash->spi, &cmd, 1, &sr, 1);
	if (ret) {
		DEBUG_INIT_S("SF: fail to write status register\n");
		return ret;
	}

	ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
	if (ret < 0) {
		DEBUG_INIT_S("SF: write status register timed out\n");
		return ret;
	}

	return 0;
}

/*
 * The following table holds all device probe functions
 *
 * shift:  number of continuation bytes before the ID
 * idcode: the expected IDCODE or 0xff for non JEDEC devices
 * probe:  the function to call
 *
 * Non JEDEC devices should be ordered in the table such that
 * the probe functions with best detection algorithms come first.
 *
 * Several matching entries are permitted, they will be tried
 * in sequence until a probe function returns non NULL.
 *
 * IDCODE_CONT_LEN may be redefined if a device needs to declare a
 * larger "shift" value.  IDCODE_PART_LEN generally shouldn't be
 * changed.  This is the max number of bytes probe functions may
 * examine when looking up part-specific identification info.
 *
 * Probe functions will be given the idcode buffer starting at their
 * manu id byte (the "idcode" in the table below).  In other words,
 * all of the continuation bytes will be skipped (the "shift" below).
 */
#define IDCODE_CONT_LEN 0
#define IDCODE_PART_LEN 5
static const struct {
	const u8 shift;
	const u8 idcode;
	struct spi_flash *(*probe) (struct spi_slave *spi, u8 *idcode);
} flashes[] = {
    { 0, 0xc8, spi_flash_probe_gigadevice, },   // used by Siklu  MV_SIKLU_WIGIG_BOARD
};
#define IDCODE_LEN (IDCODE_CONT_LEN + IDCODE_PART_LEN)

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode)
{
    //DEBUG_INIT_S("Entering spi_flash_probe\n");
	struct spi_slave *spi = &slave_global;
	struct spi_flash *flash = NULL;
	int ret, i, shift;
	u8 idcode[IDCODE_LEN], *idp;
    // Values of following vars are taken from debug outputs when running "serdes_cfg" cmd
	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	ret = spi_claim_bus(spi);
	if (ret) {
		DEBUG_INIT_S("SF: Failed to claim SPI bus\n");
		goto err_claim_bus;
	}

	/* Read the ID codes */
	//printf("SF: CMD_READ_ID 0%x %s %s %d\n", CMD_READ_ID,__FILE__, __FUNCTION__, __LINE__);
	ret = spi_flash_cmd(spi, CMD_READ_ID, idcode, sizeof(idcode));
	if (ret)
    {
		DEBUG_INIT_S("SF: Error reading ID\n");
		goto err_read_id;
    }

#ifdef DEBUG
	printf("SF: Got idcodes %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	print_buffer(0, idcode, 1, sizeof(idcode), 0);
#endif

	/* count the number of continuation bytes */
	for (shift = 0, idp = idcode;
	     shift < IDCODE_CONT_LEN && *idp == 0x7f;
	     ++shift, ++idp)
		continue;

	/* search the table for matches in shift and id */
	for (i = 0; i < ARRAY_SIZE(flashes); ++i)
		if (flashes[i].shift == shift && flashes[i].idcode == *idp) {
			/* we have a match, call probe */
			flash = flashes[i].probe(spi, idp);
			if (flash)
				break;
		}

	if (!flash) {
		DEBUG_INIT_S("SF: Unsupported manufacturer\n");
		goto err_manufacturer_probe;
	}

	DEBUG_INIT_S("SF: Detected flash\n");

	spi_release_bus(spi);

	return flash;

err_manufacturer_probe:
err_read_id:
	spi_release_bus(spi);
err_claim_bus:
	spi_free_slave(spi);
	return NULL;
}
