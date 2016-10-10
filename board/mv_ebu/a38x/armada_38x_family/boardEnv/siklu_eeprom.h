/*
 * siklu_eeprom.h
 *
 *  Created on: Jul 12, 2016
 *      Author: edwardk
 */

#ifndef SIKLU_EEPROM_H_
#define SIKLU_EEPROM_H_

#define SYS_EEPROM_MAJOR_VER        0x0010
#define SYS_EEPROM_START_MINOR_VER1 0x11 // all ver1 headers have minor ver >= this value.
//                                          the define should never be changed
#define SYS_EEPROM_MINOR_VER1       0x12 // current version >= SYS_EEPROM_START_MINOR_VER1

typedef enum // see also bsp/lib/common/flash_api.c
{
	PROTECTED_FLASH_TYPE_BACKWARD_COMPATIBLE = 0, // original
	PROTECTED_FLASH_TYPE_EMPTY_FIELD = 0,
	PROTECTED_FLASH_TYPE_MAC = 1, // original
	PROTECTED_FLASH_TYPE_BB_SERIAL_NUMBER = 2, // original this is BB board serial number, not system !!!
	PROTECTED_FLASH_TYPE_PADDING = 3, // original
	PROTECTED_FLASH_TYPE_SYSTEM_SERIAL_NUMBER = 0x10, // system serial number. New !!!
	PROTECTED_FLASH_TYPE_PRODUCT_NAME = 0x11, // product name - string null terminated
	PROTECTED_FLASH_TYPE_NETWORK_PORT_TYPE = 0x12, //  - string null terminated
	PROTECTED_FLASH_TYPE_ASSEMBLY_TYPE = 0x13, // assembly type - string null terminated
	PROTECTED_FLASH_TYPE_CRC = 0x4F, // new
	PROTECTED_FLASH_TYPE_EOF = 0xFF
// 0xFF which is unburned flash
} T_BOARD_SYS_INFO_T_FIELD_TYPE;

#pragma pack(1)
// convert string to u16, u32
typedef union {
	__u32 val;
	struct {
		char c[4];
	} u;
} struct32;

typedef union {
	__u16 va1;
	struct {
		char c[2];
	} u;
} struct16;

typedef struct {
	struct32 control_key;
	struct16 major;
	struct16 minor;
	struct32 num_tlv;
	struct32 data_size;
	struct32 crc;
} seeprom_header_S;

#pragma pack()

#define NVRAM_MAC_SIZE 6
#define NVRAM_CRC32_FIELD_SIZE 4 // unsigned int = 4 bytes
#define NVRAM_NETW_PORT_TYPE_FIELD_SIZE 4 // 4 bytes, one for each port

#define SYS_EEPROM_CRC_SEED_PRIM_VAL	(~0)
#define SYS_EEPROM_CONTROL_KEY_VAL 	0xabc1def2

typedef enum {
	SYS_EEPROM_AREA_STATUS_UNKNOWN = -2,
	SYS_EEPROM_AREA_NOT_VALID = -1,
	SYS_EEPROM_AREA_FORMAT_V0 = 0, // v0 uses 10byte field length
	SYS_EEPROM_AREA_FORMAT_V1 = 1, // v1 uses 12byte field length
} SYS_EEPROM_AREA_STATUS_E;

extern __u32 get_syseeprom_area_snor_offs(void);
extern void convert_mac_address(__u8* mac, const char *string);
extern struct spi_flash *_get_spi_flash_data(void);
extern void repair_string_printable(char* str);
extern int seeprom_erase_all_section(void);
extern char* protect_string(char* str, int size);
extern int is_port_type_valid(char port);


int seeprom_get_mac_v1(__u8* mac_addr);
int seeprom_set_mac_v1(__u8* mac_addr);

int seeprom_set_baseband_serial_v1(char* val);
int seeprom_get_baseband_serial_v1(char* val);

int seeprom_get_system_serial_v1(char* val);
int seeprom_set_system_serial_v1(char* val);

int seeprom_set_product_name_v1(char* product_name);
int seeprom_get_product_name_v1(char* product_name);

int seeprom_set_assembly_type_v1(char* assembly);
int seeprom_get_assembly_type_v1(char* assembly);

const char* seeprom_get_netw_port_map_v1(void);
int seeprom_set_netw_port_map_v1(const char*);


int seeprom_primary_format_v1(void);

#endif /* SIKLU_EEPROM_H_ */
