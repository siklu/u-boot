
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6UL 14x14 EVK board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 *
 * See also sdk_nxp/infra/u-boot-2017.11/configs/mx6ull_14x14_skl_defconfig
 */
#ifndef __MX6ULLEVK_CONFIG_H
#define __MX6ULLEVK_CONFIG_H


#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include "mx6_common.h"
#include <asm/mach-imx/gpio.h>


#define CONFIG_LIB_RAND /* required for CONFIG_NET_RANDOM_ETHADDR */

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE 0x4000
#endif
#endif



#define CONFIG_CMD_MTDPARTS	/* Enable MTD parts commands */
#define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
#define MTDIDS_DEFAULT		"nand0=gpmi-nand" //      SPI  ;spi1=spi_flash
#define MTDPARTS_DEFAULT   "mtdparts=gpmi-nand:128k(env),128k(env2),128k(env_t),40M(uimage0),"  \
    "40M(uimage1)," "16M(conf),-(log)"

#define CONFIG_NAND_ECC_BCH

#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */
#define CONFIG_APBH_DMA
#define CONFIG_APBH_DMA_BURST
#define CONFIG_APBH_DMA_BURST8

#ifndef CONFIG_CMD_SAVEENV
# define CONFIG_CMD_SAVEENV
#endif /* CONFIG_CMD_SAVEENV */

/* environment starts here  */
#define CONFIG_ENV_OFFSET             0 /*  nand_get_env_offs() */

/* should be complient to ENV_SIZE in tools/env/fw_env.c!   */
#define CONFIG_ENV_SIZE			0x10000 /* 1 sector 128k  */
#define CONFIG_ENV_RANGE		0x20000	/*  */

#define CONFIG_ENV_ADDR                 CONFIG_ENV_OFFSET
#define CONFIG_SYS_MONITOR_BASE         0x20000
#define CONFIG_SYS_MONITOR_LEN          0x20000           /* Reserve 512 kB for Monitor */


/* #define  CONFIG_LAST_STAGE_INIT */

/**************      NAND and NAND file-system related declarations     ***************/

/* follow define means that SW images stored over NAND commands, no UBI
  11.02.2018 after solve a problem we return to store uimage in UBI */
/* #define SW_PARTS_IS_PURE_NAND_PARTITIONS 1 */

#ifndef SW_PARTS_IS_PURE_NAND_PARTITIONS
	/* #define CONFIG_SYS_USE_UBI */
	#define CONFIG_CMD_UBI
	/* #define CONFIG_CMD_UBIFS   do not support ubifs in uboot ???   */
	#define CONFIG_CMD_UBIFS
	/*#define CONFIG_UBI_SILENCE_MSG disable this define prints additional logs on terminal. remove it later. edikk */
	#define CONFIG_UBI_SILENCE_MSG

#endif /* !SW_PARTS_IS_PURE_NAND_PARTITIONS */

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_RBTREE
#define CONFIG_LZO


/**************      NAND and NAND file-system related declarations  end   ***************/


#define CONFIG_CMD_DATE
/*
 * RTC
 */
#ifdef CONFIG_CMD_DATE
# define CONFIG_SYS_RTC_BUS_NUM		0
# define CONFIG_RTC_PCF8523
# define CONFIG_SYS_I2C_RTC_ADDR	0x68
#endif /* CONFIG_CMD_DATE */


#define CONFIG_IMX_THERMAL   /* used for measure internal CPU temperature */
#define CONFIG_TMP421_THERMAL   /* used for measure board temperature */

#ifdef  CONFIG_TMP421_THERMAL
# define CONFIG_SYS_TEMP_SENS_BUS_NUM		0
# define CONFIG_SYS_I2C_TEMP_SENS_ADDR		0x4c
#endif



#ifdef CONFIG_CMD_NET

/* already defined in defconfig
# define CONFIG_CMD_PING
# define CONFIG_CMD_DHCP
*/
# define CONFIG_CMD_MII
# define CONFIG_FEC_MXC
# define CONFIG_MII
# define CONFIG_FEC_ENET_DEV		0
# define CONFIG_NET_RANDOM_ETHADDR  // if no SIKLU eth addr, generate random
# define CONFIG_FEC_FIXED_SPEED			100 /* No autoneg, fix Gb */

# if (CONFIG_FEC_ENET_DEV == 0)
#  define IMX_FEC_BASE			ENET_BASE_ADDR
#  define CONFIG_FEC_MXC_PHYADDR          0x2
#  define CONFIG_FEC_XCV_TYPE             RMII
# elif (CONFIG_FEC_ENET_DEV == 1)
#  define IMX_FEC_BASE			ENET2_BASE_ADDR
#  define CONFIG_FEC_MXC_PHYADDR		0x1
#  define CONFIG_FEC_XCV_TYPE		RMII
# endif
# define CONFIG_ETHPRIME			"FEC"

# define CONFIG_PHYLIB

# define CONFIG_PHY_FIXED  /* NXP SoC connected direct to SOHO Switch by 100M FD connection */
# define SIKLU_PCB19x_SWITCH_MDIO_BUS 1 /*  PCB19x requires switch 2 MDIO busses on ENET1   */
#endif

#define PHYS_SDRAM_SIZE	SZ_512M

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

#define CONFIG_MXC_GPIO

#define CONFIG_SPI
/* #define CONFIG_CMD_SPI  defined in sdk_nxp/infra/u-boot-2017.11/configs/mx6ull_14x14_skl_defconfig*/
/* #define CONFIG_FSL_IMX_ESPI  */
/* #define CONFIG_FSL_ESPI      */
#define CONFIG_MXC_SPI
/*#define CONFIG_SOFT_SPI   not used on siklu */




#ifdef CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
/* #define CONFIG_MXC_SPI  */

#define CONFIG_SYS_FLASH_CFI            /* Flash memory is CFI compliant */
#define CONFIG_FLASH_CFI_DRIVER         /* Use drivers/cfi_flash.c */
#define CONFIG_SYS_MAX_FLASH_BANKS	1   /* max num of flash banks	*/
#define CONFIG_SYS_FLASH_BASE		0

#define CONFIG_SPI_FLASH_GIGADEVICE
#define CONFIG_SF_DEFAULT_BUS		2
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED		10000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0

#endif


#define CONFIG_SYS_MAX_NAND_DEVICE	1

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_BASE

/* MMC Configs*/
#undef CONFIG_MMC
#undef CONFIG_FSL_USDHC
#ifdef CONFIG_FSL_USDHC
# define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR
/* NAND pin conflicts with usdhc2 */
# ifdef CONFIG_SYS_USE_NAND
#  define CONFIG_SYS_FSL_USDHC_NUM	1
# else
#  define CONFIG_SYS_FSL_USDHC_NUM	2
# endif
#endif

/* I2C configs */
#ifdef CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_SPEED		100000
#endif


/* Miscellaneous configurable options */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x8000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)


#define CONFIG_BOOTCOMMAND "siklu_boot"


#define CONFIG_IOMUX_LPSR

#endif
