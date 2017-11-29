/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6UL 14x14 EVK board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __MX6ULLEVK_CONFIG_H
#define __MX6ULLEVK_CONFIG_H


#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include "mx6_common.h"
#include <asm/mach-imx/gpio.h>

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE 0x4000
#endif
#endif



#ifdef CONFIG_SPI_BOOT /* follow section is specific for siklu board */
# define SIKLU_BOARD 1   /* EVK board boot from SD card, instead it Siklu board boot from SPI NOR FLASH   */
# define CONFIG_CMD_SF

# define CONFIG_CMD_MTDPARTS	/* Enable MTD parts commands */
# define CONFIG_MTD_DEVICE	/* needed for mtdparts commands */
# define MTDIDS_DEFAULT		"nand0=armada-nand" //      SPI  ;spi1=spi_flash
# define MTDPARTS_DEFAULT   "mtdparts=armada-nand:128k(env_ro),128k(env_var0),128k(env_var1),128k(hdr0),40M(uimage0),"  \
    "128k(hdr1),40M(uimage1)," "16M(conf),-(log)"


#ifndef CONFIG_CMD_NAND
# define CONFIG_CMD_NAND
#endif
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


/* #define CONFIG_SYS_USE_UBI */
#define CONFIG_CMD_UBI
/* #define CONFIG_CMD_UBIFS   do not support ubifs in uboot   */
#define CONFIG_UBI_SILENCE_MSG
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTDPARTS
#define CONFIG_RBTREE
#define CONFIG_LZO

#endif /* CONFIG_SPI_BOOT      end section is specific for siklu board    */




#ifdef CONFIG_CMD_NET
/* already defined in defconfig
# define CONFIG_CMD_PING
# define CONFIG_CMD_DHCP
*/
# define CONFIG_CMD_MII
# define CONFIG_FEC_MXC
# define CONFIG_MII
# define CONFIG_FEC_ENET_DEV		1

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
# define CONFIG_PHY_MICREL_KSZ8XXX
# define CONFIG_PHY_MICREL_KSZ90X1

#endif





#define PHYS_SDRAM_SIZE	SZ_512M

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

#define CONFIG_MXC_GPIO


#ifdef CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
/* #define CONFIG_MXC_SPI  */
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED		20000000
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#endif


#define CONFIG_SYS_MAX_NAND_DEVICE	1

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */
#ifdef CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* NAND pin conflicts with usdhc2 */
#ifdef CONFIG_SYS_USE_NAND
#define CONFIG_SYS_FSL_USDHC_NUM	1
#else
#define CONFIG_SYS_FSL_USDHC_NUM	2
#endif
#endif

/* I2C configs */
#ifdef CONFIG_CMD_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_SPEED		100000
#endif

#ifndef SIKLU_BOARD
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"console=ttymxc0\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=imx6ull-14x14-evk.dtb\0" \
	"fdt_addr=0x83000000\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"videomode=video=ctfb:x:480,y:272,depth:24,pclk:108695,le:8,ri:4,up:2,lo:4,hs:41,vs:10,sync:0,vmode:0\0" \
	"mmcdev="__stringify(CONFIG_SYS_MMC_ENV_DEV)"\0" \
	"mmcpart=" __stringify(CONFIG_SYS_MMC_IMG_LOAD_PART) "\0" \
	"mmcroot=" CONFIG_MMCROOT " rootwait rw\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
	"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
		"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootz; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootz; " \
		"fi;\0" \

#define CONFIG_BOOTCOMMAND \
	   "mmc dev ${mmcdev};" \
	   "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadbootscript; then " \
			   "run bootscript; " \
		   "else " \
			   "if run loadimage; then " \
				   "run mmcboot; " \
			   "else run netboot; " \
			   "fi; " \
		   "fi; " \
	   "else run netboot; fi"

#endif /* SIKLU_BOARD */

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


#ifndef SIKLU_BOARD   /* for EVK  */
/* environment organization */
# define CONFIG_SYS_MMC_ENV_DEV		1	/* USDHC2 */
# define CONFIG_SYS_MMC_ENV_PART		0	/* user area */
# define CONFIG_MMCROOT			"/dev/mmcblk1p2"  /* USDHC2 */

# define CONFIG_ENV_SIZE		SZ_8K
# define CONFIG_ENV_OFFSET		(12 * SZ_64K)
#endif /* SIKLU_BOARD */

#define CONFIG_IMX_THERMAL

#define CONFIG_IOMUX_LPSR

#define CONFIG_SOFT_SPI




#endif
