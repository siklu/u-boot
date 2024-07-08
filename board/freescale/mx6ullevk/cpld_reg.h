/*
 *  Siklu portfolio products, CPLD version 1 B/C 6
 *  CPLD Registers and bit fields
 *
 *  Verified against "CPLD Register Description.xlsx",
 *      06/06/2022, 07:34:14, Moshe Levy
 *
 *  This file should be identical in two places:
 *      portfolio:host/kernel_dd/cpld_dd/cpld-nxp/cpld_reg.h
 *      board/freescale/mx6ullevk/cpld_reg.h
 *
 *  Created on: Jan 23, 2018, Author: noama
 *
 *  The following structs are width 8, LSB to MSB
 *  A register macro is followed by a union defining the fields.
 */

#ifndef CPLD_REG_H_
#define CPLD_REG_H_

#define R_CPLD_LOGIC_MAJOR                  0x0
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_cpld_major_version:8;
	} s;
} T_CPLD_LOGIC_MAJOR_REGS;


#define R_CPLD_LOGIC_MINOR_BOARDTYPE        0x1
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_cpld_board_type:4;
		u8 cfg_cpld_minor_version:4;
	} s;
} T_CPLD_LOGIC_MINOR_BOARDTYPE_REGS;


#define R_CPLD_LOGIC_RESET_CONTROL          0x2
typedef union
{
	u8 uint8;
	struct
	{
		/* Active low SW reset */
		u8 cfg_sw_rst:1;

		/* setting this bit to '1' enables the clear on read of the
		 * cause register (address 0x3), default state is '0' because it
		 * used as asynchronic reset to the cause register. (power-up on
		 * time reset). SW should change this bit to '1' after boot and
		 * only then, clear the cause register by reading address 0x3.
		 * If SW will not enable this bit before clearing the causexi
		 * register the RST-LED might malfunction when user presses it.
		 */
		u8 cfg_cause_reg_clear_on_read_en:1;

		/* Active low reset to main PLL */
		u8 cfg_pll_rst_n:1;

		/* Active low reset to 10G PHY */
		u8 cfg_ten_g_rst_n:1;

		/* Not used */
		u8 cfg_phy1_rst_c:1;

		u8 cfg_sprom_flash_wp_n:1;

		/* Active low reset to modem */
		u8 cfg_mdm_rst_n:1;

		u8 cfg_wdi_cnt_en:1;
	} s;
} T_CPLD_LOGIC_RESET_CONTROL_REGS;


#define R_CPLD_LOGIC_RESET_CAUSE            0x3
typedef union
{
	u8 uint8;
	struct
	{
		/* Indicates that the system woke up for the first time */
		u8 cfg_startup_rst:1;

		/* The CPLD had issued reset (through the push button), because
		 * the SW didn't activate the WDI after startup */
		u8 cfg_no_sw_wdi_rst:1;

		/* note to clear this indication with reading address 0x9.
		 * (power indications) */
		u8 cfg_power_fail_rst:1;

		/* indicates WD reset */
		u8 cfg_wd_rst:1;

		/* Indicates that the reset push button (active low) was pushed
		 * for less than 5 sec. If the push button is pressed, the CPLD
		 * issues reset to the board. */
		u8 cfg_ext_pb_short_rst:1;

		/* Indicates that the reset push button (active low) was pushed
		 * for more than 5 sec. If the push button is pressed, the CPLD
		 * issues reset to the board. */
		u8 cfg_ext_pb_long_rst:1;

		/* Indicates that the SW instructed the CPLD to reset the board,
		 * through register TBD */
		u8 cfg_cpu_rst:1;

		u8 cfg_ext_pb_medium_rst:1;
	} s;
} T_CPLD_LOGIC_RESET_CAUSE_REGS;


#define R_CPLD_LOGIC_MISC_STATUS            0x4
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_rssi_indication:1;	/* RSSI exist */
		u8 padding0:1;
		u8 cfg_poe_exist:1;		/* Standard Poe detected */
		u8 cfg_usb_fault:1;		/* USB fault detected */
		u8 padding1:1;
		u8 cfg_nand_ry_by:1;		/* NAND Ready/Busy */
		u8 padding2:2;
	} s;
} T_CPLD_LOGIC_MISC_STATUS_REGS;


#define R_CPLD_LOGIC_WD_RW                  0x5
typedef union
{
	u8 uint8;
	struct
	{
		/* When asserted to '1' it selects a dummy clock to the WD
		 * device on board. As a result, It cancels the WD operation.
		 * (Software WD triggers no longer necessary, reset will not be
		 * done to the board upon missing SW WD) -- This is not an
		 * operational mode -- ONLY for debug */
		u8 cfg_wd_src:1;

		/* Strobe the WD reset device. When writing 1 to this register,
		 * the CPLD issues a single 20us pulse to the WD device */
		u8 cfg_wd_ignore:1;

		u8 cfg_wd_allow_10_sec:1;   /* not documented bit */
		u8 padding0:1;
		u8 cfg_wd_wdi:1;

		/* R/W: Set to 1 to assert RESET to switch PCB */
		u8 cfg_fpga_8020_rst:1;

		/* R: (Not yet implemented) Signal coming from EH8020 B2B
		 * (reserved for asserting reset from switch side) */
		u8 from_fpga_8020_rst:1;

		u8 padding1:1;
	} s;
} T_CPLD_LOGIC_WD_RW_REGS;


#define R_CPLD_LOGIC_UNUSED_0X6             0x6

#define R_CPLD_LOGIC_SFP_MODE               0x7
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_sfp_mod_def:1;		/* 10G SFP + exist */
		u8 padding0:3;
		u8 cfg_sfp_los_signal:1;	/* 10G SFP + Los of Signal */
		u8 padding1:3;
	} s;
} T_CPLD_LOGIC_SFP_MODE_REGS;


#define R_CPLD_LOGIC_DIP_MODE               0x8
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_dip:4;
		u8 cfg_poe_pair1_exist:1;
		u8 cfg_poe_pair2_exist:1;
		u8 cfg_pll_lol_n:1;		/* PLL Loss of Lock */
		u8 padding0:1;
	} s;
} T_CPLD_LOGIC_DIP_MODE_REGS;


#define R_CPLD_LOGIC_MODEM_LEDS_CTRL        0x9
typedef union
{
	u8 uint8;
	struct
	{
		/* Modem led control,
		 * 0 - Green,
		 * 1 - Blinking green,
		 * 2 - Yellow,
		 * 3 - Off */
		u8 cfg_modem_led:2;	/* bits 0:1 */

		u8 padding0:1;		/* bit 2 */

		/* Active low reset to AES 10G PHY (MACSec) */
		u8 cfg_aes_phy_rst_n:1;	/* bit 3 */

		u8 cfg_10g_board:1;	/* bit 4 */
		u8 cfg_fpga_rst_n:1;	/* bit 5 AES FPGA */

		/* Active low reset to 10G SERDES */
		u8 cfg_tlk_rst_n:1;	/* bit 6 */

		/* Active low reset to L2 Switch */
		u8 cfg_switch_rst_n:1;	/* bit 7 */
	} s;
} T_CPLD_LOGIC_MODEM_LEDS_CTRL_REGS;


#define R_CPLD_LOGIC_ETHERNET_LEDS_CTRL     0xA
typedef union
{
	u8 uint8;
	struct
	{
        u8 cfg_eth1_led_color:1;    // 0 - green , 1 - yellow
        u8 cfg_eth2_0_led_color:1;  // 0 - green , 1 - yellow
        u8 cfg_eth2_1_led_color:1;  // 0 - green , 1 - yellow
        u8 cfg_xpic_led_color:1;    // 0 - green , 1 - yellow
        u8 cfg_dummy_1:1;           // 0 - not used
        u8 cfg_eth2_led_state:1;    // 0 - on , 1 - off
        u8 cfg_eth3_led_state:1;    // 0 - on , 1 - off
        u8 cfg_xpic_led_state:1;    // 0 - on , 1 - off
	} s;
} T_CPLD_LOGIC_ETHERNET_LEDS_CTRL_REGS;


#define R_CPLD_LOGIC_POWER_LEDS_CTRL        0xB
typedef union
{
	u8 uint8;
	struct
	{
		// 0 - Red ,
		// 1 - Blink Red,
		// 2 - Green,
		// 3 - Blink Green,
		// 4 - Off,
		// 5 - Red/Green,
		// 6 - Red pulse,
		// 7 - Green pulse
		u8 cfg_power_led:3;

		u8 cfg_max_pll_mux_sel1:5;
	} s;
} T_CPLD_LOGIC_POWER_LEDS_CTRL_REGS;


#define R_CPLD_LOGIC_POWER_STATUS           0xC
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_pg_l2_1v5:1;
		u8 cfg_pg_2v5:1;
		u8 cfg_pg_l2_1v1:1;
		u8 cfg_pg_phy_1v8:1;
		u8 cfg_pg_1v35_ddr:1;
		u8 cfg_pg_sd_1v0:1;
		u8 cfg_pg_modem_core:1;
		u8 cfg_pg_phy_0v8:1;
	} s;
} T_CPLD_LOGIC_POWER_STATUS_REGS;

#define R_CPLD_LOGIC_UNISED_0XD             0xD

#define R_CPLD_LOGIC_INT_HNDLR_0            0xE
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_sfp_los_s:1;
		u8 padding0:3;
		u8 cfg_sfp_mod_def0:1;
		u8 padding1:3;
	} s;
} T_CPLD_LOGIC_INT_HNDLR_0_REGS;


#define R_CPLD_LOGIC_INT_HNDLR_1            0xF
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_phy10g_int:1;
		u8 cfg_sfp_txfault:1;
		u8 padding0:2;
		u8 cfg_switch_int:1;
		u8 cfg_nsync1588:1;
		u8 cfg_rtc_int:1;
		u8 cfg_modem_int:1;
	} s;
} T_CPLD_LOGIC_INT_HNDLR_1_REGS;


#define R_CPLD_LOGIC_INT_HNDLR_0_MASK       0x10
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_sfp_los_s_mask:1;
		u8 padding0:3;
		u8 cfg_sfp_mod_def_mask:1;
		u8 padding1:3;
	} s;
} T_CPLD_LOGIC_INT_HNDLR_0_MASK_REGS;


#define R_CPLD_LOGIC_INT_HNDLR_1_MASK       0x11
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_phy10g_int_mask:1;
		u8 cfg_sfp_txfault_mask:1;
		u8 padding0:2;
		u8 cfg_switch_mask:1;
		u8 cfg_nsync1588_mask:1;
		u8 cfg_rtc_mask:1;
		u8 cfg_modem_mask:1;
	} s;
} T_CPLD_LOGIC_INT_HNDLR_1_MASK_REGS;


#define R_CPLD_LOGIC_CPLD_INT_CAUSE_RO      0x12
typedef union
{
	u8 uint8;
	struct
	{
		u8 cpld_int_cause_ro:2;
		u8 padding0:6;
	} s;
} T_CPLD_LOGIC_CPLD_INT_CAUSE_RO_REGS;


#define R_CPLD_LOGIC_MISC_0                 0x13
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_int_hndlr_0_global_enable:1;
		u8 cfg_int_hndlr_1_global_enable:1;
		u8 padding0:6;
	} s;
} T_CPLD_LOGIC_MISC_0_REGS;


#define R_CPLD_LOGIC_MISC_1                 0x14
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_sfp_tx_disable:1;
		u8 padding0:5;
		u8 cfg_usb_start:1;	/* 1 = enable USB chip */
		u8 padding1:1;
	} s;
} T_CPLD_LOGIC_MISC_1_REGS;


#define R_CPLD_TEST                         0x15 /* read back inverted write
						    value */


#define R_CPLD_LOGIC_MODE_TEST              0x16
typedef union
{
	u8 uint8;
	struct
	{
		/* (RW) Set to 1 for EH8020, 0 for EH8010 */
		u8 cfg_mode_8020:1;

		/* (RW) Set to 1 to put LED and SFP control in bypass mode (i.e.
		 * controlled from CPLD) */
		u8 cfg_led_tst_mode_8020:1;

		/* LED test contols (RW)
		 * bit 2 - RJ45 1G control
		 * bit 3 - RJ45 10G contorl
		 * bit 4 - SFP 10G control
		 * bit 5 - spare control */
		u8 cfg_sfp_test_faultcfg_led_tst_val_8020:4;
		u8 padding0:2;
	} s;
} T_CPLD_LOGIC_MODE_TEST;


#define R_CPLD_LOGIC_UNUSED_0X17            0x17
#define R_CPLD_LOGIC_UNUSED_0X18            0x18
#define R_CPLD_LOGIC_UNUSED_0X19            0x19
#define R_CPLD_LOGIC_UNUSED_0X1A            0x1A
#define R_CPLD_LOGIC_UNUSED_0X1B            0x1B

#define R_CPLD_LOGIC_GPIO                   0x1C
typedef union
{
	u8 uint8;
	struct
	{
		u8 gpio70:4;
		u8 gpio80:4;
	} s;
} T_CPLD_LOGIC_GPIO_REGS;


#define R_CPLD_LOGIC_CFG_SEL_MISC           0x1D
typedef union
{
	u8 uint8;
	struct
	{
		/* Select TX 70/80:
		 * 0 - 70
		 * 1 - 80
		 * 2 - Loopback
		 * 3 - Reserved */
		u8 cfg_rf_tx_sel:2;

		/* Select RX 70/80:
		 * 0 - 70
		 * 1 - 80
		 * 2 - Loopback
		 * 3 - Reserved */
		u8 cfg_rf_rx_sel:2;

		u8 cfg_aux_a2d2_sel2:1;
		u8 cfg_clk_out_test_sel:2;
		u8 padding0:1;
	} s;
} T_CPLD_LOGIC_CFG_SEL_MISC_REGS;

#define R_CPLD_LOGIC_UNUSED_0x1E            0x1E
#define R_CPLD_LOGIC_UNUSED_0x1F            0x1F
#define R_CPLD_LOGIC_UNUSED_0x20            0x20
#define R_CPLD_LOGIC_UNUSED_0x21            0x21
#define R_CPLD_LOGIC_UNUSED_0x22            0x22

#define R_CPLD_LOGIC_HW_ASM_VER             0x23
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_hw_asm_ver:8;	/* Read HW assembly resistors */
	} s;
} T_CPLD_LOGIC_HW_ASM_VER_REGS;


#define R_CPLD_LOGIC_MISC_2                 0x24
typedef union
{
	u8 uint8;
	struct
	{
		u8 cfg_serial_nor_flash_wp_n:1;
		u8 padding0:7;
	} s;
} T_CPLD_LOGIC_MISC_2_REGS;


#define R_CPLD_LOGIC_UNUSED_0X25            0x25
#define R_CPLD_LOGIC_UNUSED_0X26            0x26

#define R_CPLD_LOGIC_SER_EEPROM_IF          0x27
typedef union
{
	uint8_t u8;
	struct
	{
		u8 cfg_eeprom_70_dir:1;
		u8 cfg_eeprom_70_data_out:1;
		u8 cfg_eeprom_70_data_in:1;
		u8 cfg_eeprom_80_dir:1;
		u8 cfg_eeprom_80_data_out:1;
		u8 cfg_eeprom_80_data_in:1;
		u8 padding0:2;
	} s;
} T_CPLD_LOGIC_SER_EEPROM_IF_REGS;


#define R_CPLD_LOGIC_SER_EEPROM_WRITE_DATA  0x2E
#define R_CPLD_LOGIC_SER_EEPROM_CMD         0x2F
#define R_CPLD_LOGIC_SER_EEPROM_ADDR_LSB    0x30
#define R_CPLD_LOGIC_SER_EEPROM_ADDR_MSB    0x31
#define R_CPLD_LOGIC_SER_EEPROM_READ_DATA   0x32


#define R_CPLD_LOGIC_SER_EEPROM_STATUS      0x33
typedef union
{
	u8 uint8;
	struct
	{
		u8 eeprom_slave_err:1;
		u8 eeprom_master_err:1;
		u8 eeprom_nextlsb:1;
		u8 eeprom_ready:1;
		u8 padding0:4;
	} s;
} T_CPLD_LOGIC_SER_EEPROM_STATUS;


#define R_CPLD_LOGIC_SER_EEPROM_CTL         0x34
typedef union
{
	u8 uint8;
	struct
	{
		u8 eeprom_addr_needed:1;
		u8 eeprom_write_readnot:1;
		u8 eeprom_trig:1;
		u8 eeprom_next:1;		// for sequential reads
		u8 eeprom_is_70:1;
		u8 padding0:3;
	} s;
} T_CPLD_LOGIC_SER_EEPROM_CTRL;

#endif /* CPLD_REG_H_ */
