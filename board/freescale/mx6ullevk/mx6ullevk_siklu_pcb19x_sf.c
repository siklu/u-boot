/*
 * mx6ullevk_siklu_pcb19x_sf.c
 *
 *  Created on: Jan 28, 2018
 *      Author: edwardk
 *
 *      the file based on sdk_nxp/infra/u-boot-2017.11/env/sf.c
 *
 */

/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * (C) Copyright 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <environment.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>
#include <search.h>
#include <errno.h>
#include <dm/device-internal.h>

#include "siklu_def.h"
#include "siklu_api.h"

#undef CONFIG_ENV_SECT_SIZE
#define CONFIG_ENV_SECT_SIZE  (SIKLU_SYSEEPROM_SF_SIZE)

DECLARE_GLOBAL_DATA_PTR;

struct spi_flash *siklu_get_env_flash(void) {
	static struct spi_flash *env_flash = NULL;
	if (!env_flash) {
		env_flash = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
		CONFIG_SF_DEFAULT_CS,
		CONFIG_SF_DEFAULT_SPEED,
		CONFIG_SF_DEFAULT_MODE);
		if (!env_flash) {
			printf("%s: spi_flash_probe() failed\n", __func__);
			return NULL;
		}
	}
	return env_flash;
}


/*
 *
 */
int siklu_sf_sys_eeprom_read(const char* buf, int size) {

	int rc;

	rc = spi_flash_read(siklu_get_env_flash(), SIKLU_SYSEEPROM_SF_OFFS, size,
			(char*) buf);
	if (rc != 0) {
		printf("SYSEEPROM SF area read error rc %d\n", rc);
	}
	return rc;
}

/*
 *
 */
int siklu_sf_sys_eeprom_write(const char* buf, int size) {

	int rc;
	// printf("%s() called, line %d\n", __func__, __LINE__); // edikk remove

	printf("Erasing SPI flash...");
	rc = spi_flash_erase(siklu_get_env_flash(), SIKLU_SYSEEPROM_SF_OFFS,
	SIKLU_SYSEEPROM_SF_SIZE);
	if (rc) {
		printf("SYSEEPROM SF area erase error rc %d\n", rc);
		return rc;
	}
	printf("Ok\n");
	rc = spi_flash_write(siklu_get_env_flash(), SIKLU_SYSEEPROM_SF_OFFS, size,
			(char*) buf);
	if (rc != 0) {
		printf("SYSEEPROM SF area write error rc %d\n", rc);
	}
	return rc;
}


int siklu_sf_sys_eeprom_erase(void)
{
	int rc = 0;
	puts("Erasing SPI flash...");
	rc = spi_flash_erase(siklu_get_env_flash(), SIKLU_SYSEEPROM_SF_OFFS,
	SIKLU_SYSEEPROM_SF_SIZE);
	if (rc) {
		printf("SYSEEPROM SF area erase error rc %d\n", rc);
		return rc;
	}
	return rc;
}
