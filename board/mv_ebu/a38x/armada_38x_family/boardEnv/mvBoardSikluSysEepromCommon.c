/*
 * mvBoardSikluSysEepromCommon.c
 *
 *  Created on: Jul 12, 2016
 *      Author: edwardk
 */

#include <common.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <crc.h>
#include <malloc.h>

#include <spi_flash.h>
#include "siklu_eeprom.h"

extern struct spi_flash *get_spi_flash_data(void);

//DECLARE_GLOBAL_DATA_PTR;

typedef struct {

	int (*get_mac)(__u8* mac_addr);
	int (*set_mac)(__u8* mac_addr);

	int (*set_baseband_serial)(char* val);
	int (*get_baseband_serial)(char* val);

	int (*get_system_serial)(char* val);
	int (*set_system_serial)(char* val);

	int (*set_product_name)(char* product_name);
	int (*get_product_name)(char* product_name);

	const char* (*get_netw_port_map)(void);
	int (*set_netw_port_map)(const char*);

    int (*set_assembly_type)(char* type);
    int (*get_assembly_type)(char* type);

	int (*primary_format)(void);

}seeprom_hndlr_S;


static seeprom_hndlr_S* GetHndlr(void) {
	static seeprom_hndlr_S* seeprom_hndlr = NULL;
	SYS_EEPROM_AREA_STATUS_E status;

	if (seeprom_hndlr)
		return seeprom_hndlr;

	seeprom_hndlr = malloc(sizeof(seeprom_hndlr_S));
	if (!seeprom_hndlr)
		return NULL;

	status = SYS_EEPROM_AREA_FORMAT_V1;
	switch (status) {
	case SYS_EEPROM_AREA_FORMAT_V1:

		seeprom_hndlr->get_mac = seeprom_get_mac_v1;
		seeprom_hndlr->set_mac = seeprom_set_mac_v1;

		seeprom_hndlr->set_baseband_serial = seeprom_set_baseband_serial_v1;
		seeprom_hndlr->get_baseband_serial = seeprom_get_baseband_serial_v1;

		seeprom_hndlr->get_system_serial = seeprom_get_system_serial_v1;
		seeprom_hndlr->set_system_serial = seeprom_set_system_serial_v1;

		seeprom_hndlr->get_product_name = seeprom_get_product_name_v1;
		seeprom_hndlr->set_product_name = seeprom_set_product_name_v1;

		seeprom_hndlr->get_netw_port_map = seeprom_get_netw_port_map_v1;
		seeprom_hndlr->set_netw_port_map = seeprom_set_netw_port_map_v1;

		seeprom_hndlr->get_assembly_type = seeprom_get_assembly_type_v1;
		seeprom_hndlr->set_assembly_type = seeprom_set_assembly_type_v1;

		seeprom_hndlr->primary_format = seeprom_primary_format_v1;

		break;
	default:
		printf("%s() Unknown SYSEEPROM AREA format %d\n", __func__, status);
		free(seeprom_hndlr);
		seeprom_hndlr = NULL;

	}
	return seeprom_hndlr;
}

__u32 get_syseeprom_area_snor_offs(void) {
	// sNOR size = 2M, last 64kB SYSEEPROM area

	const __u32 syseeprom_area_snor_offset = (2 * 1024 - 64) * 1024;
	return syseeprom_area_snor_offset;
}

int is_port_type_valid(char port) {
	switch (port) {
	case 'c':
	case 'f':
	case '-':
		return 1;
		break;
	default:
		return 0;
		break;
	}
	return 0;
}

/**
 * set_mac_address - stores a MAC address into the EEPROM
 *
 * This function takes a pointer to MAC address string
 * (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number) and
 * stores it in one of the MAC address fields of the EEPROM local copy.
 */
void convert_mac_address(__u8* mac, const char *string)
{
	char *p = (char *) string;
	unsigned int i;
	if (!string)
	{
		printf("Usage: mac <n> XX:XX:XX:XX:XX:XX\n");
		return;
	}
	for (i = 0; *p && (i < 6); i++)
	{
		mac[i] = simple_strtoul(p, &p, 16);
		if (*p == ':')
		p++;
	}
}

/*
 * quick and dirty access
 */
struct spi_flash *_get_spi_flash_data(void) {
	struct spi_flash *flash = get_spi_flash_data();
	if (flash == NULL) {       // need call probe
		char *argv[] = { NULL };
		extern int do_spi_flash_probe(int argc, char * const argv[]);
		do_spi_flash_probe(1, argv);
		flash = get_spi_flash_data();
	}
	return flash;
}

/*
 *
 * check and replace non printable characters
 */
void repair_string_printable(char* str) {
	int i;
	for (i = 0; str[i]; i++) {
		if (str[i] != '-') // also '-' acceptable
			if (!isalnum(str[i])) {
				str[i] = '.';
			}
	}
}

static char repair_char_printable(char _char) {
	char p = _char;

	if (p != '-') // also '-' acceptable
			{
		if (!isalnum(p))
			p = '.';
	}
	return p;
}

int seeprom_erase_all_section(void) {
	int rc = 0;

	// erase here whole SYSEEPROM sector in sNOR
	rc = spi_flash_erase(_get_spi_flash_data(), get_syseeprom_area_snor_offs(),
			_get_spi_flash_data()->sector_size);
	return rc;
}

char* protect_string(char* str, int size) {
	static char temp[100]; // take size enough large
	memset(temp, 0, sizeof(temp));
	if (size >= sizeof(temp))
		size = sizeof(temp) - 1;
	memcpy(temp, str, size);
	return temp;
}

/*
 * read header
 * check control key
 * read major and minor version
 * decide status
 */
static int seeprom_check_data_validity(void) {
	static int rc = -1;
	seeprom_header_S header;
	__u8 *pseeprom = (__u8 *) &header;

	if ((rc >= 0)) // read only once
		return rc;

	memset(pseeprom, 0x00, sizeof(header));

	size_t len = sizeof(header);
	rc = spi_flash_read(_get_spi_flash_data(), get_syseeprom_area_snor_offs(),
			len, pseeprom);
	if (rc != 0) {
		printf(" Read Serial EEPROM FAIL\n");
		rc = -1;
		return rc;
	}

	if (header.control_key.val != SYS_EEPROM_CONTROL_KEY_VAL) {
		printf(" Control Key is Wrong. Expected 0x%x, Read 0x%x\n",
		SYS_EEPROM_CONTROL_KEY_VAL, header.control_key.val);
		rc = -1;
		return rc;
	}

	if (header.major.va1 != SYS_EEPROM_MAJOR_VER) {
		printf(" Wrong major version. Expected 0x%x, Read 0x%x\n",
				header.major.va1, SYS_EEPROM_MAJOR_VER);
		rc = -1;
		return rc;
	}
	return 0;
}

int siklu_get_mac_from_seeprom(__u8* mac_addr)
{
	seeprom_hndlr_S* hndlr = NULL;
	int rc;

	rc = seeprom_check_data_validity();
	if (rc >= 0)
	hndlr = GetHndlr();
	if (hndlr)
		return hndlr->get_mac(mac_addr);
	else
		return -1;
}

/*
 * should be called after init sNOR and SYSEEPROM functionality!
 *   returns number of ethernet ports according to syseeprom configuration string
 */
int siklu_get_seeprom_net_number_eth_ports(void) {
	int i;
	int rc = 0;
	const char* p;

	seeprom_hndlr_S* hndlr = NULL;

	if (rc >= seeprom_check_data_validity())
		hndlr = GetHndlr();

	if (hndlr) {
		p = hndlr->get_netw_port_map();     // seeprom_get_netw_port_map();
		for (i = 0; i < NVRAM_NETW_PORT_TYPE_FIELD_SIZE; i++) {
			if ((p[i] == 'c') || (p[i] == 'f')) {
				rc++;
			}
		}
	}
	return rc;
}
/*
 *  called on power up before first SYS EEPROM access
 */
void siklu_prepare_syseeprom(void) {
	int rc = seeprom_check_data_validity();

	if (rc >= 0) { // ok we have data
		return;
	}
	// no SYSEEPROM data recognized, make default format
	seeprom_hndlr_S* hndlr = GetHndlr(); // prepare area
	if (hndlr) {
		hndlr->primary_format();
	}
}

/******************************************************************
 *                      CLI
 *
 ******************************************************************/
static int display_syseeprom_raw_data(void) {
	int lines;
	__u8 data[0x100];
	__u8* p = data;
	int offset = 0;

	size_t len = sizeof(data);
	spi_flash_read(_get_spi_flash_data(), get_syseeprom_area_snor_offs(), len,
			data);
	for (lines = 0; lines < 8; lines++) // read 8 * 16 bytes
			{
		offset = lines * 16;
		//
		printf(
				"[%04x]: %02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x  -  ",
				offset, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8],
				p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		printf(
				"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", //
				repair_char_printable(p[0]), repair_char_printable(p[1]),
				repair_char_printable(p[2]), //
				repair_char_printable(p[3]), repair_char_printable(p[4]),
				repair_char_printable(p[5]), //
				repair_char_printable(p[6]), repair_char_printable(p[7]),
				repair_char_printable(p[8]), //
				repair_char_printable(p[9]), repair_char_printable(p[10]),
				repair_char_printable(p[11]), //
				repair_char_printable(p[12]), repair_char_printable(p[13]),
				repair_char_printable(p[14]), //
				repair_char_printable(p[15]));
		p += 0x10;
	}
	return 0;
}

/*
 *
 *
 *
 */
static int do_maintenance_sys_serial_eeprom(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[]) //
{
	int rc = 0;

	if (argc == 1) {
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if (strcmp(argv[1], "r") == 0) { // read raw data
		display_syseeprom_raw_data();
		return 0;
	} //
	if (strcmp(argv[1], "e") == 0) {
		seeprom_erase_all_section();
		return 0;
	}

	seeprom_hndlr_S* hndlr = NULL;

	if (rc >= seeprom_check_data_validity())
		hndlr = GetHndlr();

	if (hndlr == NULL) {
		printf("%s()  Error on line %d\n", __func__, __LINE__);
		return 1;
	}

	if (strcmp(argv[1], "f") == 0) // parse SEEPROM data
			{
		__u8 mac[NVRAM_MAC_SIZE];
		char data[30];
		hndlr->get_mac(mac);
		printf("MAC               %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0],
				mac[1], mac[2], mac[3], mac[4], mac[5]);

		hndlr->get_baseband_serial(data);
		printf("Board Serial      %s\n", data);

		hndlr->get_system_serial(data);
		printf("System Serial     %s\n", data);

		hndlr->get_product_name(data);
		printf("Product name      %s\n", data);

		const char* p = hndlr->get_netw_port_map();
		printf("Network port type %c%c%c%c\n", p[0], p[1], p[2], p[3]);

		hndlr->get_assembly_type(data);
        printf("Assembly type     %s\n", data);


	} //
	else if (strcmp(argv[1], "w") == 0) // write default parameters
			{
		seeprom_erase_all_section(); // SPI NOR FLASH should be erased before write
		mdelay(10);
		hndlr->primary_format();
		printf("\nReboot required before continue!\n");

	} else if (strcmp(argv[1], "b") == 0) // set baseband serial
			{
		hndlr->set_baseband_serial(argv[2]);
	} else if (strcmp(argv[1], "m") == 0) // set MAC
			{
		hndlr->set_mac((__u8 *) argv[2]);
	} else if (strcmp(argv[1], "n") == 0) // set network's port types
			{
		hndlr->set_netw_port_map(argv[2]);
	} else if (strcmp(argv[1], "p") == 0) // set product name
			{
		hndlr->set_product_name(argv[2]);
	}
	else if (strcmp(argv[1], "a") == 0) // set assembly board type string
            {
        hndlr->set_assembly_type(argv[2]);
    }
	else {
		printf("Unknown parameter\n");
		return cmd_usage(cmdtp);
	}

	return 0;
}

U_BOOT_CMD(sseepro, 7, 1, do_maintenance_sys_serial_eeprom, "Read/Maintenance System Serial EEPROM raw data",
		"r - read raw data, f - display fields, w - primary format, e - erase, \n  \
        b - set baseband serial, m - set MAC, n - set Ethernet port map, p - set Product Name,\
		a - set assembly board type");

