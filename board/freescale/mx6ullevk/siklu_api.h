/*
 * siklu_api.h
 *
 *  Created on: Aug 29, 2017
 *      Author: edwardk
 */

#ifndef SIKLU_API_H_
#define SIKLU_API_H_


extern int siklu_is_restore2fact_default(void);
extern const char* siklu_mutable_env_get(const char *varname);

extern uint32_t get_nand_part_offset_by_name(const char* name);
extern int siklu_board_init(void);
extern int siklu_board_late_init(void);


extern int siklu_cpld_read(u8 reg, u8* data);
extern int siklu_cpld_write(u8 reg, u8 data);

extern int siklu_88e639x_reg_read(u8 port, u8 reg, u16* val);
extern int siklu_88e639x_reg_write(u8 port, u8 reg, u16 val);


typedef enum {
	MODULE_RFIC_70,
	MODULE_RFIC_80,
} MODULE_RFIC_E;

extern int siklu_rfic_module_read (MODULE_RFIC_E module, u8 reg, u8* data);
extern int siklu_rfic_module_write(MODULE_RFIC_E module, u8 reg, u8  data);

typedef enum {
	SIKLU_MDIO_BUS0, // SOHO
	SIKLU_MDIO_BUS1, // 10G PHY and TI Transceiver
} SIKLU_MDIO_BUS_E;

extern int siklu_mdio_bus_connect(SIKLU_MDIO_BUS_E bus);

extern int siklu_sf_sys_eeprom_read(const char* buf, int size);
extern int siklu_sf_sys_eeprom_write(const char* buf, int size);
extern int siklu_sf_sys_eeprom_erase(void);

// SYSEEPROM related API
extern int siklu_syseeprom_init(void);
extern int siklu_syseeprom_display(void);
extern int siklu_syseeprom_restore_default(void);
extern int siklu_syseeprom_get_val(const char* name, char* val);
extern int  siklu_syseeprom_set_val(const char* name, const char* val);
extern int  siklu_syseeprom_udate(void);

#endif /* SIKLU_API_H_ */
