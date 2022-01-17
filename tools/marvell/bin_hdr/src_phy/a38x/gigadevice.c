/*
 * gigadevice.c
 *
 *  Created on: Oct 9, 2016
 *      Author: edwardk
 *
 *      Used on Siklu boards MV_SIKLU_WIGIG_BOARD
 */
#include <bootstrap_os.h>
#include <common.h>
#include <mvUart.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

struct gigadevice_flash_params {
    u16 idcode;
    u16 nr_blocks;
    u8  addr_cycles;
    const char *name;
};

static const struct gigadevice_flash_params gigadevice_flash_table[] = {
    {
        .idcode = 0x4014,
        .nr_blocks = 128,
        .addr_cycles = 3,
        .name = "GD25Q08",
    },
    {
        .idcode = 0x4015,
        .nr_blocks = 256,
        .addr_cycles = 3,
        .name = "GD25Q16",
    },
};

/*
 * Code below reference copied from Siklu Cavium CVMX UBOOT project
 */
//Gigadevice
//{"GD25Q08",        0xc84014, 0x0,   64 * 1024,   128,          SECT_4K},  // added by SIKLU,
//{"GD25Q16",        0xc84015, 0x0,   64 * 1024,   256,          SECT_4K},  // added by SIKLU,

// Macronix
//{"MX25L8006",      0xc22014, 0x0,   64 * 1024,    16,                0},  // SIKLU
//{"M25P64",         0x202017, 0x0,   64 * 1024,   128,                0},  // default on EVB SFF
//{"MX25L1606",      0xc22015, 0x0,   64 * 1024,    32,                0}, // added by SIKLU,



/*
 *
 *
 */
struct spi_flash spi_flash_global;

struct spi_flash *spi_flash_probe_gigadevice(struct spi_slave *spi, u8 *idcode)
{
    //DEBUG_INIT_S("SF: Entering spi_flash_probe_gigadevice\n");
    const struct gigadevice_flash_params *params;
    struct spi_flash *flash;
    unsigned int i;
    u16 id = idcode[2] | idcode[1] << 8;

    for (i = 0; i < ARRAY_SIZE(gigadevice_flash_table); i++) {
        params = &gigadevice_flash_table[i];
        if (params->idcode == id)
            break;
    }

    if (i == ARRAY_SIZE(gigadevice_flash_table)) {
        DEBUG_INIT_S("SF: Unsupported Gygadevice ID ");
        DEBUG_INIT_D(id, 8);
        DEBUG_INIT_S("\n");
        return NULL;
    }

    flash = &spi_flash_global;
    //if (!flash) {
    //    DEBUG_INIT_S("SF: Failed to allocate memory\n");
    //    return NULL;
    //}

    flash->spi = spi;
    flash->name = params->name;

    flash->write = spi_flash_cmd_write_multi;
    flash->erase = spi_flash_cmd_erase;
    flash->read = spi_flash_cmd_read_fast;
    flash->page_size = 256;
    flash->sector_size = 256 * 16 * 16;
    flash->size = flash->sector_size * params->nr_blocks;
    flash->addr_cycles=params->addr_cycles;
    /* Clear BP# bits for read-only flash */
    spi_flash_cmd_write_status(flash, 0);

    return flash;
}
