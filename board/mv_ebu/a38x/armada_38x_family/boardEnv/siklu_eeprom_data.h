/*
 * siklu_eeprom_data.h
 *
 *  Created on: Jul 12, 2016
 *      Author: edwardk
 */

#ifndef SIKLU_EEPROM_DATA_H_
#define SIKLU_EEPROM_DATA_H_

#pragma pack(1)

typedef struct //
{
	__u8 t;
	__u8 l;
	__u8 mac[NVRAM_MAC_SIZE];
} mac_tlv_S;

typedef struct //
{
	__u8 t;
	__u8 l;
	char serial[NVRAM_SERIAL_NUM_FIELD_MAX_SIZE];
} serial_tlv_S;

typedef struct //
{
	__u8 t;
	__u8 l;
	char product[NVRAM_PRODUCT_NAME_FIELD_SIZE];
} product_tlv_S;

typedef struct //
{
	__u8 t;
	__u8 l;
	char port_type[NVRAM_NETW_PORT_TYPE_FIELD_SIZE];
} netw_port_tlv_S;

typedef struct //
{
    __u8 t;
    __u8 l;
    char type[NVRAM_ASSEMBLY_TYPE_FIELD_SIZE];
} assembly_type_tlv_S;

typedef struct {
	product_tlv_S product_name;
	mac_tlv_S mac;
	serial_tlv_S serial_bb; // base board
	serial_tlv_S serial_sys; // serial system
	netw_port_tlv_S netw_port;
	assembly_type_tlv_S assembly; // this field is only for Marvell platform
} seeprom_data_S;

typedef struct {
	seeprom_header_S header; // header's crc doesn't cover tcontrol_S !
	seeprom_data_S data;
} seeprom_board_info_S; // header and data

typedef struct {
	seeprom_board_info_S seeprom_board_info;
} seeprom_S;

#pragma pack()

#endif /* SIKLU_EEPROM_DATA_H_ */
