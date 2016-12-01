/*
 * mvBoardSikluSysEepromApi.c
 *
 *  Created on: Jul 12, 2016
 *      Author: edwardk
 */

#include <common.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <crc.h>

#include <spi_flash.h>

#include "siklu_board_system.h"

#define ETH_ALEN	6
#define NVRAM_NETW_PORT_TYPE_FIELD_SIZE 4


extern char *getenv_se(const char *name);

int seeprom_get_mac_v1(__u8* mac_addr)
{
    int  rc = -1;

    const char *se_val = getenv_se("SE_mac");
    if (se_val)
    {
        char mac_ascii[20];
        char *mp = mac_ascii;
        unsigned i;

        strncpy(mac_ascii, se_val, sizeof(mac_ascii));
        mac_ascii[sizeof(mac_ascii) - 1] = 0;
        for (i = 0; i < ETH_ALEN; i++)
        {
            char *d = strsep(&mp, ":");
            if (d)
                mac_addr[i] = (__u8)simple_strtoul(d, NULL, 16);
        }
        rc = 0;
    }
    else
        memset(mac_addr, 0, ETH_ALEN);
    return rc;
}

int seeprom_get_system_serial_v1(char* val)
{
    const char *se_val = getenv_se("SE_board_serial");
    int rc = 0;
    if (se_val)
        strcpy(val, se_val);
    else
        rc = -1;
    return rc;
}

int seeprom_get_assembly_type_v1(char* assembly)
{
    const char *se_val = getenv_se("SE_assembly");
    int rc = 0;
    if (se_val)
        strcpy(assembly, se_val);
    else
        rc = -1;
    return rc;
}

SIKLU_NETWORK_PORT_TYPE_E siklu_get_network_port_type(int port_num)
{
    SIKLU_NETWORK_PORT_TYPE_E type = SIKLU_NETWORK_PORT_TYPE_NONE;
    const char *se_val = getenv_se("SE_port_map");
    if (se_val)
    {
        if (se_val[port_num] == 'c')
            type = SIKLU_NETWORK_PORT_TYPE_COPPER;
        else if (se_val[port_num] == 'f')
            type = SIKLU_NETWORK_PORT_TYPE_FIBER;
    }
    return type;
}
