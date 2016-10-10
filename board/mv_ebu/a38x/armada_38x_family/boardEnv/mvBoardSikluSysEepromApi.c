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
#include "siklu_eeprom.h"

// follow declarations should be before "siklu_eeprom_data.h" file!

#define NVRAM_SERIAL_NUM_FIELD_MAX_SIZE 20
#define NVRAM_PRODUCT_NAME_FIELD_SIZE   20 // field size limited to 30 bytes
#define AUX_A2D_CALIB_FLOAT_STRING_LEN  15
#define NVRAM_ASSEMBLY_TYPE_FIELD_SIZE  20 // field size limited to 30 bytes

#include "siklu_eeprom_data.h"

//DECLARE_GLOBAL_DATA_PTR;

static seeprom_S siklu_mrv_system_seeprom;

static int seeprom_write2nvram_new_data_v1(void) {
	int rc = 0;

	// after complete  init all TLV set header info
	siklu_mrv_system_seeprom.seeprom_board_info.header.crc.val = crc32(
			(unsigned int) SYS_EEPROM_CRC_SEED_PRIM_VAL,
			(unsigned char *) &siklu_mrv_system_seeprom.seeprom_board_info.data,
			(int) sizeof(seeprom_data_S));

	// last step - write data to seeprom
	seeprom_erase_all_section();  // SPI NOR FLASH should be erased before write
	mdelay(100);
	rc = spi_flash_write(_get_spi_flash_data(), get_syseeprom_area_snor_offs(),
			sizeof(seeprom_S), &siklu_mrv_system_seeprom);
	if (rc != 0) {
		printf("%s(): ERROR write data\n", __func__);
		return -1;
	}

	return rc;
}

/*
 *
 *
 */
static int seeprom_read_fields_from_eeprom_v1(void) //
{
	unsigned char *pseeprom = (unsigned char*) &siklu_mrv_system_seeprom;
	int rc = 0;
	__u32 crc;
	static int is_already_read = 0;

	if ((is_already_read)) // read only once
		return 0;

	memset(pseeprom, 0x00, sizeof(seeprom_S));

	size_t len = sizeof(seeprom_S);
	rc = spi_flash_read(_get_spi_flash_data(), get_syseeprom_area_snor_offs(),
			len, pseeprom);

	if (rc != 0) {
		printf(" Read Serial EEPROM FAIL\n");
		return -1;
	}
	// Step 2 - analyze header
	if (siklu_mrv_system_seeprom.seeprom_board_info.header.control_key.val
			!= SYS_EEPROM_CONTROL_KEY_VAL) {
		printf(" Control Key is Wrong. Expected 0x%x, Read 0x%x\n",
		SYS_EEPROM_CONTROL_KEY_VAL,
				siklu_mrv_system_seeprom.seeprom_board_info.header.control_key.val);
		return -1;
	}

	// calculate and compare CRC
	crc =
			crc32(SYS_EEPROM_CRC_SEED_PRIM_VAL,
					(__u8 *) &siklu_mrv_system_seeprom.seeprom_board_info.data,
					(int) siklu_mrv_system_seeprom.seeprom_board_info.header.data_size.val);
	if (crc != siklu_mrv_system_seeprom.seeprom_board_info.header.crc.val) {
		printf(
				"%s() Wrong CRC. Exists 0x%x, Calculated 0x%x, seeprom_raw_data %p, calculated size %d\n",
				__func__,
				siklu_mrv_system_seeprom.seeprom_board_info.header.crc.val, crc,
				&siklu_mrv_system_seeprom, (int) sizeof(seeprom_data_S));
		return -1;
	}

	is_already_read = 1;
	return rc;
}

/*
 *
 */
int seeprom_get_mac_v1(__u8* mac_addr)
{
	int rc = seeprom_read_fields_from_eeprom_v1();
	if (rc == 0)
	{
		memcpy(mac_addr, siklu_mrv_system_seeprom.seeprom_board_info.data.mac.mac, NVRAM_MAC_SIZE);
	}
	else
	{
		printf("SEPROM data is wrong, write default values\n");
		seeprom_primary_format_v1();

		seeprom_read_fields_from_eeprom_v1();
		memcpy(mac_addr, siklu_mrv_system_seeprom.seeprom_board_info.data.mac.mac,
		NVRAM_MAC_SIZE);
	}
	return 0;
}

/*
 *
 *
 */
int seeprom_primary_format_v1(void) {
	// seeprom_S seeprom;
	unsigned char *pseeprom = (unsigned char*) &siklu_mrv_system_seeprom;
	char* tmp;
	int rc = 0;

	printf("Primary format System EEPROM Ver 1. Please wait... ");

	memset(pseeprom, 0x00, sizeof(seeprom_S));

	siklu_mrv_system_seeprom.seeprom_board_info.header.control_key.val =
	SYS_EEPROM_CONTROL_KEY_VAL;
	siklu_mrv_system_seeprom.seeprom_board_info.header.major.va1 =
			SYS_EEPROM_MAJOR_VER;
	siklu_mrv_system_seeprom.seeprom_board_info.header.minor.va1 =
			SYS_EEPROM_MINOR_VER1;

	// Set MAC
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.t =
			PROTECTED_FLASH_TYPE_MAC;
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.l = NVRAM_MAC_SIZE;
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.mac[0] = 0x00;
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.mac[1] = 0x24;
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.mac[2] = 0xa4;
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.mac[3] = 0x00;
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.mac[4] = 0xde;
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.mac[5] = 0xad;

	// Set base board serial
	siklu_mrv_system_seeprom.seeprom_board_info.data.serial_bb.t =
			PROTECTED_FLASH_TYPE_BB_SERIAL_NUMBER;
	siklu_mrv_system_seeprom.seeprom_board_info.data.serial_bb.l =
			NVRAM_SERIAL_NUM_FIELD_MAX_SIZE;
	tmp = "233333332";
	memcpy(siklu_mrv_system_seeprom.seeprom_board_info.data.serial_bb.serial,
			tmp, strlen(tmp));

	// Set System serial
	siklu_mrv_system_seeprom.seeprom_board_info.data.serial_sys.t =
			PROTECTED_FLASH_TYPE_SYSTEM_SERIAL_NUMBER;
	siklu_mrv_system_seeprom.seeprom_board_info.data.serial_sys.l =
			NVRAM_SERIAL_NUM_FIELD_MAX_SIZE;
	tmp = "";
	memcpy(siklu_mrv_system_seeprom.seeprom_board_info.data.serial_sys.serial,
			tmp, strlen(tmp));

	// Set Network port types
	siklu_mrv_system_seeprom.seeprom_board_info.data.netw_port.t =
			PROTECTED_FLASH_TYPE_NETWORK_PORT_TYPE;
	siklu_mrv_system_seeprom.seeprom_board_info.data.netw_port.l =
			NVRAM_NETW_PORT_TYPE_FIELD_SIZE;
	strcpy(siklu_mrv_system_seeprom.seeprom_board_info.data.netw_port.port_type,
			"ccf");

	// Set Product Name
	siklu_mrv_system_seeprom.seeprom_board_info.data.product_name.t =
			PROTECTED_FLASH_TYPE_PRODUCT_NAME;
	siklu_mrv_system_seeprom.seeprom_board_info.data.product_name.l =
	NVRAM_PRODUCT_NAME_FIELD_SIZE;
	strcpy(
			siklu_mrv_system_seeprom.seeprom_board_info.data.product_name.product,
			"EH700");

	// Set assembly type
    siklu_mrv_system_seeprom.seeprom_board_info.data.assembly.t =
            PROTECTED_FLASH_TYPE_ASSEMBLY_TYPE;
    siklu_mrv_system_seeprom.seeprom_board_info.data.assembly.l =
    NVRAM_ASSEMBLY_TYPE_FIELD_SIZE;
    strcpy(
            siklu_mrv_system_seeprom.seeprom_board_info.data.assembly.type,   "FABB031A");

	// after complete  init all TLV set header info
	siklu_mrv_system_seeprom.seeprom_board_info.header.num_tlv.val = 6;
	siklu_mrv_system_seeprom.seeprom_board_info.header.data_size.val =
			sizeof(seeprom_data_S);

	rc = seeprom_write2nvram_new_data_v1();

	printf(" Ok\n");
	return rc;
}

int seeprom_get_system_serial_v1(char* val) {
	int rc = 0;
	rc = seeprom_read_fields_from_eeprom_v1();
	if (rc == 0) {
		memcpy(val,
				siklu_mrv_system_seeprom.seeprom_board_info.data.serial_sys.serial,
				NVRAM_SERIAL_NUM_FIELD_MAX_SIZE);
	} else {
		printf("%s() SEPROM data is wrong, write default values\n", __func__);
		seeprom_primary_format_v1();
		seeprom_read_fields_from_eeprom_v1();
		memcpy(val,
				siklu_mrv_system_seeprom.seeprom_board_info.data.serial_sys.serial,
				NVRAM_SERIAL_NUM_FIELD_MAX_SIZE);
	}
	return rc;
}

int seeprom_set_system_serial_v1(char* val) {
	int rc = 0;
	printf("%s() Should never be called!\n", __func__);
	return rc;
}

int seeprom_get_baseband_serial_v1(char* val) {
	int rc = 0;
	rc = seeprom_read_fields_from_eeprom_v1();
	if (rc == 0) {
		memcpy(val,
				siklu_mrv_system_seeprom.seeprom_board_info.data.serial_bb.serial,
				NVRAM_SERIAL_NUM_FIELD_MAX_SIZE);
	} else {
		printf("%s() SEPROM data is wrong, write default values\n", __func__);
		seeprom_primary_format_v1();
		seeprom_read_fields_from_eeprom_v1();
		memcpy(val,
				siklu_mrv_system_seeprom.seeprom_board_info.data.serial_bb.serial,
				NVRAM_SERIAL_NUM_FIELD_MAX_SIZE);
	}
	return rc;
}

/*
 * input: board serial number string, up to NVRAM_SERIAL_NUM_FIELD_MAX_SIZE digits
 */
int seeprom_set_baseband_serial_v1(char* val) {
	int rc = 0;

	seeprom_read_fields_from_eeprom_v1();
	if (strlen(val) > NVRAM_SERIAL_NUM_FIELD_MAX_SIZE) {
		printf("Baseband serial number too large: %d\n",
				NVRAM_SERIAL_NUM_FIELD_MAX_SIZE);
		return 0;
	}
	// Set base board serial
	siklu_mrv_system_seeprom.seeprom_board_info.data.serial_bb.t =
			PROTECTED_FLASH_TYPE_BB_SERIAL_NUMBER;
	siklu_mrv_system_seeprom.seeprom_board_info.data.serial_bb.l =
			NVRAM_SERIAL_NUM_FIELD_MAX_SIZE;
	memset(siklu_mrv_system_seeprom.seeprom_board_info.data.serial_bb.serial, 0,
			NVRAM_SERIAL_NUM_FIELD_MAX_SIZE);
	memcpy(siklu_mrv_system_seeprom.seeprom_board_info.data.serial_bb.serial,
			val, strlen(val));

	rc = seeprom_write2nvram_new_data_v1();

	return rc;
}

/*
 * input string in form AA:BB:CC:DD:EE:FF
 */
int seeprom_set_mac_v1(__u8* val)
{
	int rc = 0;
	__u8* m = siklu_mrv_system_seeprom.seeprom_board_info.data.mac.mac;
	rc = seeprom_read_fields_from_eeprom_v1();

	if (!(isxdigit(val[0]) && isxdigit(val[1]) //
			&& isxdigit(val[3]) && isxdigit(val[4])//
			&& isxdigit(val[6]) && isxdigit(val[7])//
			&& isxdigit(val[9]) && isxdigit(val[10])//
			&& isxdigit(val[12]) && isxdigit(val[13])//
			&& isxdigit(val[15]) && isxdigit(val[16])))
	{
		printf(" The string includes non-hex characters\n");
		return 0;
	}

	memset(m, 0, NVRAM_MAC_SIZE);
	convert_mac_address(m, (const char*)val);

	printf(" New MAC Address %02X%02X%02X%02X%02X%02X\n", m[0], m[1], m[2], m[3], m[4], m[5]);

	// Set MAC
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.t = PROTECTED_FLASH_TYPE_MAC;
	siklu_mrv_system_seeprom.seeprom_board_info.data.mac.l = NVRAM_MAC_SIZE;

	rc = seeprom_write2nvram_new_data_v1();

	return rc;
}

int seeprom_get_product_name_v1(char* product_name) {
	int rc = 0;
	rc = seeprom_read_fields_from_eeprom_v1();
	if (rc == 0) {
		memcpy(product_name,
				siklu_mrv_system_seeprom.seeprom_board_info.data.product_name.product,
				NVRAM_PRODUCT_NAME_FIELD_SIZE);
	} else {
		printf("%s() SEPROM data is wrong, write default values\n", __func__);
		seeprom_primary_format_v1();
		seeprom_read_fields_from_eeprom_v1();
		memcpy(product_name,
				siklu_mrv_system_seeprom.seeprom_board_info.data.product_name.product,
				NVRAM_PRODUCT_NAME_FIELD_SIZE);
	}
	return rc;
}

int seeprom_set_product_name_v1(char* product_name) {
	int rc = 0;

	seeprom_read_fields_from_eeprom_v1();

	if (strlen(product_name) > NVRAM_PRODUCT_NAME_FIELD_SIZE) {
		printf("Product name too long: %d\n", NVRAM_PRODUCT_NAME_FIELD_SIZE);
		return 0;
	}
	siklu_mrv_system_seeprom.seeprom_board_info.data.product_name.t =
			PROTECTED_FLASH_TYPE_PRODUCT_NAME;
	siklu_mrv_system_seeprom.seeprom_board_info.data.product_name.l =
			NVRAM_PRODUCT_NAME_FIELD_SIZE;
	memset(
			siklu_mrv_system_seeprom.seeprom_board_info.data.product_name.product,
			0, NVRAM_PRODUCT_NAME_FIELD_SIZE);
	strcpy(
			siklu_mrv_system_seeprom.seeprom_board_info.data.product_name.product,
			product_name);

	rc = seeprom_write2nvram_new_data_v1();
	return rc;
}

/*
 *
 *
 */
int seeprom_set_assembly_type_v1(char* assembly) { // edikk
    int rc = 0;

    seeprom_read_fields_from_eeprom_v1();

    if (strlen(assembly) > NVRAM_ASSEMBLY_TYPE_FIELD_SIZE ) {
        printf("Assembly type string too long: %d\n", NVRAM_ASSEMBLY_TYPE_FIELD_SIZE);
        return 0;
    }
    siklu_mrv_system_seeprom.seeprom_board_info.data.assembly.t =
            PROTECTED_FLASH_TYPE_ASSEMBLY_TYPE;
    siklu_mrv_system_seeprom.seeprom_board_info.data.assembly.l =
            NVRAM_ASSEMBLY_TYPE_FIELD_SIZE;
    memset(
            siklu_mrv_system_seeprom.seeprom_board_info.data.assembly.type,
            0, NVRAM_ASSEMBLY_TYPE_FIELD_SIZE);
    strcpy(
            siklu_mrv_system_seeprom.seeprom_board_info.data.assembly.type,
            assembly);

    rc = seeprom_write2nvram_new_data_v1();


    return rc;
}
/*

*/
int seeprom_get_assembly_type_v1(char* assembly) {
    int rc = 0;
    rc = seeprom_read_fields_from_eeprom_v1();
    if (rc == 0) {
        memcpy(assembly,
                siklu_mrv_system_seeprom.seeprom_board_info.data.assembly.type,
                NVRAM_ASSEMBLY_TYPE_FIELD_SIZE );
    } else {
        printf("%s() SEPROM data is wrong, write default values\n", __func__);
        seeprom_primary_format_v1();
        seeprom_read_fields_from_eeprom_v1();
        memcpy(assembly,
                siklu_mrv_system_seeprom.seeprom_board_info.data.assembly.type,
                NVRAM_ASSEMBLY_TYPE_FIELD_SIZE);
    }
    return rc;
}


/*
 * input string exactly 4 characters:
 *      'c' - copper
 *      'f' - fiber
 *      '-' - non-existant port
 */
int seeprom_set_netw_port_map_v1(const char* port_map) {
	int rc = 0;
	rc = seeprom_read_fields_from_eeprom_v1();

	if (strlen(port_map) != 4) {
		printf(
				"Network port map should be exact 4 characters lens, received %d\n",
				strlen(port_map));
		return 0;
	}
	if (!(is_port_type_valid(port_map[0]) && is_port_type_valid(port_map[1])
			&& is_port_type_valid(port_map[2])
			&& is_port_type_valid(port_map[3]))) {
		printf(
				"Network port map values should be 'c'=copper or 'f'=fiber or '-'=no physical port\n");
		return 0;
	}

	// update fields
	// Set Network port types
	siklu_mrv_system_seeprom.seeprom_board_info.data.netw_port.t =
			PROTECTED_FLASH_TYPE_NETWORK_PORT_TYPE;
	siklu_mrv_system_seeprom.seeprom_board_info.data.netw_port.l =
			NVRAM_NETW_PORT_TYPE_FIELD_SIZE;
	memcpy(siklu_mrv_system_seeprom.seeprom_board_info.data.netw_port.port_type,
			port_map, NVRAM_NETW_PORT_TYPE_FIELD_SIZE);

	rc = seeprom_write2nvram_new_data_v1();

	return rc;
}

const char* seeprom_get_netw_port_map_v1(void) {
	static char netw_port_map[NVRAM_NETW_PORT_TYPE_FIELD_SIZE + 1] = "----";
	char* p =
			siklu_mrv_system_seeprom.seeprom_board_info.data.netw_port.port_type;

	seeprom_read_fields_from_eeprom_v1();

	if ((is_port_type_valid(p[0]) && is_port_type_valid(p[1])
			&& is_port_type_valid(p[2]) && is_port_type_valid(p[3])))
		memcpy(netw_port_map, p, 4);
	return netw_port_map;
}
