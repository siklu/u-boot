/*
 * siklu_def.h
 *
 *  Created on: Aug 29, 2017
 *      Author: edwardk
 *
 *      Same code used in host/sysapi/sysapi-nxp/
 *      Any changes here should be reflected
 *
 */

#ifndef SIKLU_DEF_H_
#define SIKLU_DEF_H_



#define SIKLU_BIST_ENVIRONMENT_NAME "bist"
#define SIKLU_BIST_MUT_ENVIRONMENT_NAME "SK_bist" // siklu_remark002


#define PHY_AES_88x3310_DEV_ADDR 	6
#define PHY_88x3310_DEV_ADDR 		2
#define TI10031_DEV_ADDR 			4

#define CONFIG_CPLD_DEFAULT_BUS		1
#define CONFIG_CPLD_DEFAULT_CS		1
#define CONFIG_CPLD_DEFAULT_SPEED	10000000
#define CONFIG_CPLD_DEFAULT_MODE	SPI_MODE_0
#define CONFIG_CPLD_DIP_MODE_REG_ADDR 0x8

#define CONFIG_RFIC_DEFAULT_BUS			0
#define CONFIG_RFIC70_DEFAULT_CS		0
#define CONFIG_RFIC80_DEFAULT_CS		1
#define CONFIG_RFIC_DEFAULT_SPEED	1000000
#define CONFIG_RFIC_DEFAULT_MODE	SPI_MODE_0


#define SIKLU_SYSEEPROM_SF_OFFS		0x1f0000
#define SIKLU_SYSEEPROM_SF_SIZE		0x010000 // defined also in drivers/mtd/spi/sf.c

// SYSEEPROM defines
#define KEY_VAL_FIELD_SIZE 		64
#define SYSEEPROM_NUM_FIELDS 	64
#define SIKLU_SF_ENV_SIZE		0x1000 // less than sector size, limited to 4k

// PLL
# define CONFIG_SYS_PLL_BUS_NUM					0
# define CONFIG_SYS_I2C_UNBURNED_PLL_ADDR		0x69
# define CONFIG_SYS_I2C_BURNED_PLL_ADDR			0x59

#endif /* SIKLU_DEF_H_ */
