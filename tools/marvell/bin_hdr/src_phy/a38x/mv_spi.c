/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Derived from drivers/spi/mpc8xxx_spi.c
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */
#include <bootstrap_os.h>
#include <mvUart.h>
#include "mvSysSpiApi.h"
#include "mvDeviceId.h"
#include "mvSpiSpec.h"
#include "mvCtrlEnvSpec.h"

#include "mvSpi.h"
#include <spi.h>

struct spi_slave slave_global;

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				unsigned int max_hz, unsigned int mode)
{
    //DEBUG_INIT_S("Entering spi_setup_slave\n");

	struct spi_slave *slave;

	if (!spi_cs_is_valid(bus, cs))
    {
        DEBUG_INIT_S("spi is not valid\n");
		return NULL;
    }

	slave = &slave_global;
	if (!slave)
		return NULL;

	slave->bus = bus;
	slave->cs = cs;

	mvSysSpiInit(bus,max_hz);
	return slave;
}

/* Running MV_SPI_REG_WRITE in mvSpiBaudRateSet() 
 * caused the following error after returning from the bin_hdr BOOTROM extension:
    BootROM: Image checksum verification FAILED
    BootROM: Bad header at offset D4200000
    BootROM: Bad header at offset D4400000
    BootROM: Bad header at offset D4600000
    BootROM: Bad header at offset D4800000
    BootROM: Bad header at offset D4A00000
    BootROM: Bad header at offset D4C00000
    BootROM: Bad header at offset D4E00000*/

// It seems that writes to memory areas where the U-Boot image was loaded by the BOOTROM.
// As a result, when we execute SPI_REG_WRITE, it changes the U-Boot image in memory
// and causes a failure when the BOOTROM code later checks the image checksum.
// So we save the memory values that might change because of SPI_REG_WRITE,
// and restore them before exiting back to the BOOTROM.

MV_U32 intCauseRegVal;
MV_U32 dataOutRegVal ;
MV_U32 ifCtrlRegVal  ;
MV_U32 ifConfigRegVal;

void spi_save_regs()
{
    int spiId = 1;
	intCauseRegVal = MV_REG_READ(MV_SPI_INT_CAUSE_REG(spiId));
	dataOutRegVal  = MV_REG_READ(MV_SPI_DATA_OUT_REG(spiId));
	ifCtrlRegVal   = MV_REG_READ(MV_SPI_IF_CTRL_REG(spiId));
	ifConfigRegVal = MV_REG_READ(MV_SPI_IF_CONFIG_REG(spiId));
}

void spi_restore_regs()
{
    int spiId = 1;
    MV_REG_WRITE(MV_SPI_INT_CAUSE_REG(spiId), intCauseRegVal);
    MV_REG_WRITE(MV_SPI_DATA_OUT_REG(spiId), dataOutRegVal);
    MV_REG_WRITE(MV_SPI_IF_CTRL_REG(spiId), ifCtrlRegVal);
    MV_REG_WRITE(MV_SPI_IF_CONFIG_REG(spiId), ifConfigRegVal);
}

void spi_free_slave(struct spi_slave *slave)
{
	//free(slave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return ((bus <= 1) && (cs >= 0) && (cs < 8));
}

void spi_cs_activate(struct spi_slave *slave)
{
    //DEBUG_INIT_S("Entering spi_cs_activate\n");
	mvSpiCsSet(slave->bus, slave->cs);
	mvSpiCsAssert(slave->bus);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
    //DEBUG_INIT_S("Entering spi_cs_deactivate\n");
	mvSpiCsDeassert(slave->bus);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	     void *din, unsigned long flags)
{
    //DEBUG_INIT_S("Entering spi_xfer\n");
	MV_STATUS ret;
	MV_U8* pdout = (MV_U8*)dout;
	MV_U8* pdin = (MV_U8*)din;
	int tmp_bitlen = bitlen;
	MV_U8 tmp_dout = 0;

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

	/* Verify that the SPI mode is in 8bit mode */
	MV_REG_BIT_RESET(
            MV_SPI_IF_CONFIG_REG(slave->bus), 
            MV_SPI_BYTE_LENGTH_MASK);

	/* TX/RX in 8bit chanks */

	while (tmp_bitlen > 0)
	{
		if(pdout)
			tmp_dout = (*pdout) & 0xff;

		/* Transmitted and wait for the transfer to be completed */
		if ((ret = mvSpi8bitDataTxRx(slave->bus,tmp_dout, pdin)) != MV_OK)
        {
			return ret;
        }

		/* increment the pointers */
		//printf("in=[0x%x]",*pdin);
		if (pdin)
			pdin++;
		//printf("out=[0x%x]",*pdout);
		if (pdout)
			pdout++;

		tmp_bitlen-=8;
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
