/*
 * SPI flash interface
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>

static void *_memcpy(u8 *dest, const u8 *src, size_t size)
{
	unsigned char *dptr = dest;
	const unsigned char *ptr = src;
	const unsigned char *end = src + size;

	while (ptr < end)
		*dptr++ = *ptr++;

	return dest;
}




static int spi_flash_read_write(struct spi_slave *spi,
				const u8 *cmd, size_t cmd_len,
				const u8 *data_out, u8 *data_in,
				size_t data_len)
{
	//unsigned long flags = SPI_XFER_BEGIN;
	int ret;

#ifdef CONFIG_SIKLU_BOARD // edikk repair CS problem!!!!
	{
#define MAX_BUF_SIZE 0x400
		static u8 tx_buf[MAX_BUF_SIZE];
		static u8 rx_buf[MAX_BUF_SIZE];
		int ret = 0;

		memset(tx_buf,0,sizeof(tx_buf));
		memset(rx_buf,0,sizeof(rx_buf));
		if ((data_len+cmd_len)>=MAX_BUF_SIZE) {
			printf("%s() buff to short %d,%d\n", __func__, data_len+cmd_len, MAX_BUF_SIZE); // error
		}

		if (cmd) {
			_memcpy(tx_buf, cmd, cmd_len);
		}

		ret = spi_xfer(spi, (cmd_len+data_len) * 8, tx_buf, rx_buf, SPI_XFER_BEGIN | SPI_XFER_END);
		if (data_in) {
			_memcpy(data_in, &rx_buf[cmd_len], data_len);

			if (0) //edikk for debug
			{
				int i;
				printf("  RX data: (data_in %p, %p, %p) ", data_in, tx_buf, rx_buf);
				for (i=0;i<data_len;i++) {
					printf(" %02x", data_in[i]);
				}
				printf("\n");
			}
		}
		return 0;
	}


#else
	if (data_len == 0)
		flags |= SPI_XFER_END;

	ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags);
	if (ret) {
		printf("SF: Failed to send command (%zu bytes): %d\n", cmd_len, ret);
	} else if (data_len != 0) {
		ret = spi_xfer(spi, data_len * 8, data_out, data_in,
					SPI_XFER_END);
		if (ret)
			printf("SF: Failed to transfer %zu bytes of data: %d\n",
			      data_len, ret);
	}
#endif
	return ret;
}

int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	return spi_flash_read_write(spi, cmd, cmd_len, NULL, data, data_len);
}

int spi_flash_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len)
{
#ifdef CONFIG_SIKLU_BOARD  // edikk repair CS problem!!!!
	{
#undef MAX_BUF_SIZE
#define MAX_BUF_SIZE 100
		u8 tx_buf[MAX_BUF_SIZE];
		u8 rx_buf[MAX_BUF_SIZE];

		memset(tx_buf,0,sizeof(tx_buf));
		memset(rx_buf,0,sizeof(rx_buf));
		tx_buf[0] = cmd;

		if ((len+1)>=MAX_BUF_SIZE) {
			printf("%s() buff to short %d,%d\n", __func__, len, MAX_BUF_SIZE); // error
		}
		int __attribute__((unused)) ret = spi_xfer(spi, (1+len) * 8, tx_buf, rx_buf, SPI_XFER_BEGIN | SPI_XFER_END);
		memcpy(response, rx_buf+1, len);
		return 0;
	}
#else
	return spi_flash_cmd_read(spi, &cmd, 1, response, len);
#endif
}

int spi_flash_cmd_write(struct spi_slave *spi, const u8 *cmd, size_t cmd_len,
		const void *data, size_t data_len)
{
	return spi_flash_read_write(spi, cmd, cmd_len, data, NULL, data_len);
}
