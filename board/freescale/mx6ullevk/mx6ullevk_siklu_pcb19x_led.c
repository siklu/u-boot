/*
 * mx6ullevk_siklu_pcb19x_led.c
 *
 *  Created on: Feb 19, 2018
 *      Author: noama
 */


#include <common.h>
#include <command.h>
#include <version.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <linux/compiler.h>

#include "siklu_def.h"
#include "siklu_api.h"
#include "cpld_reg.h"
#include "miiphy.h"


#define SWITCH_IDENT_REG	0x16


int siklu_set_led_modem(SKL_BOARD_LED_MODE_E mode)
{
	T_CPLD_LOGIC_MODEM_LEDS_CTRL_REGS modem_leds;
	u8 rx_buf[10];
	int rc;
	memset(rx_buf, 0, sizeof(rx_buf));

	rc = siklu_cpld_read(R_CPLD_LOGIC_MODEM_LEDS_CTRL, rx_buf);
	if (rc != CMD_RET_SUCCESS)
	{
		printf("\ncpld read failed\n");
		return rc;
	}
	modem_leds = (T_CPLD_LOGIC_MODEM_LEDS_CTRL_REGS) rx_buf[3];

	switch (mode)
	{
	case SKL_LED_MODE_GREEN:
		modem_leds.s.cfg_modem_led = 0;
        break;
	case SKL_LED_MODE_GREEN_BLINK:
		modem_leds.s.cfg_modem_led = 1;
        break;
	case SKL_LED_MODE_YELLOW:
		modem_leds.s.cfg_modem_led = 2;
        break;
	case SKL_LED_MODE_OFF:
		modem_leds.s.cfg_modem_led = 3;
        break;
    default:
    	printf("Invalid modem led mode");
        return -1; // invalid mode
        break;
	}

	rc = siklu_cpld_write(R_CPLD_LOGIC_MODEM_LEDS_CTRL, modem_leds.uint8);
	return rc;
}

int siklu_set_led_power(SKL_BOARD_LED_MODE_E mode)
{
	T_CPLD_LOGIC_POWER_LEDS_CTRL_REGS pwoer_leds;
	u8 rx_buf[10];
	int rc;
	memset(rx_buf, 0, sizeof(rx_buf));

	rc = siklu_cpld_read(R_CPLD_LOGIC_POWER_LEDS_CTRL, rx_buf);
	if (rc != CMD_RET_SUCCESS)
	{
		printf("\ncpld read failed\n");
		return rc;
	}
	pwoer_leds = (T_CPLD_LOGIC_POWER_LEDS_CTRL_REGS) rx_buf[3];

	switch (mode)
	{
	case SKL_LED_MODE_YELLOW:
		pwoer_leds.s.cfg_power_led = 0;
        break;
	case SKL_LED_MODE_YELLOW_BLINK:
		pwoer_leds.s.cfg_power_led = 1;
        break;
	case SKL_LED_MODE_GREEN:
		pwoer_leds.s.cfg_power_led = 2;
        break;
	case SKL_LED_MODE_GREEN_BLINK:
		pwoer_leds.s.cfg_power_led = 3;
        break;
	case SKL_LED_MODE_OFF:
		pwoer_leds.s.cfg_power_led = 4;
        break;
    default:
    	printf("Invalid modem led mode");
        return -1; // invalid mode
        break;
	}

	rc = siklu_cpld_write(R_CPLD_LOGIC_POWER_LEDS_CTRL, pwoer_leds.uint8);
	return rc;
}

int siklu_set_cpld_eth_led_color(SKL_BOARD_LED_TYPE_E led, SKL_BOARD_LED_MODE_E mode)
{
	T_CPLD_LOGIC_ETHERNET_LEDS_CTRL_REGS eth_leds;
	u8 rx_buf[10], val;
	int rc;
	memset(rx_buf, 0, sizeof(rx_buf));

	if (mode == SKL_LED_MODE_GREEN_BLINK || mode == SKL_LED_MODE_GREEN)
		val = 0;
	else
		val = 1;

	rc = siklu_cpld_read(R_CPLD_LOGIC_ETHERNET_LEDS_CTRL, rx_buf);
	if (rc != CMD_RET_SUCCESS)
	{
		printf("\ncpld read failed\n");
		return rc;
	}
	eth_leds = (T_CPLD_LOGIC_ETHERNET_LEDS_CTRL_REGS) rx_buf[3];

	if (led == SKL_LED_ETH1)
	{
		eth_leds.s.cfg_eth1_led_color = val;
	}
	else if (led == SKL_LED_ETH2_0)
	{
		eth_leds.s.cfg_eth2_0_led_color = val;
	}
	else if (led == SKL_LED_ETH2_1)
	{
		eth_leds.s.cfg_eth2_1_led_color = val;
	}
	else
	{
    	printf("Invalid led type");
        return -1; // invalid mode
	}

	rc = siklu_cpld_write(R_CPLD_LOGIC_ETHERNET_LEDS_CTRL, eth_leds.uint8);
	return rc;
}

static int set_eth1_led_to_direct_mode(void)
{
#define SOHO_NXP_CPU_PORT 	0
#define SOHO_NXP_ETH1_PORT 	5

	u16 val;
	int rc = 0;
	static int is_init = 0;

	if (is_init)
		return rc;

	//	change to direct LED mode
	val = 0x8010;
	rc = siklu_88e639x_reg_write(SOHO_NXP_CPU_PORT, SWITCH_IDENT_REG, val);
	if (rc != 0)
	{
		printf(" Soho write ERROR %d\n", rc);
		return rc;
	}

	is_init = 1;
	return rc;
}

int siklu_set_eth1_led(SKL_BOARD_LED_MODE_E mode)
{
	int ret = CMD_RET_FAILURE;

#define SOHO_NXP_ETH1_PORT 	5

	u16 val;

	ret = set_eth1_led_to_direct_mode();
	if (ret != CMD_RET_SUCCESS)
		return -1;

	siklu_set_cpld_eth_led_color(SKL_LED_ETH1, mode);

	if (mode == SKL_LED_MODE_OFF)
	{
		val = 0x803e;
	}
	else if (mode == SKL_LED_MODE_GREEN_BLINK || mode == SKL_LED_MODE_YELLOW_BLINK)
	{
		val = 0x803d;
	}
	else if (mode == SKL_LED_MODE_GREEN || mode == SKL_LED_MODE_YELLOW)
	{
		val = 0x803f;
	}
	else
	{
		printf("Invalid led color mode\n");
		return -1;
	}


	ret = siklu_88e639x_reg_write(SOHO_NXP_ETH1_PORT, SWITCH_IDENT_REG, val);
	if (ret == 0) {
		printf(" 0x%04X\n", val);
	} else {
		printf(" Read ERROR %d\n", ret);
	}

	return ret;
}

int siklu_set_eth2_led(SKL_BOARD_LED_TYPE_E led, SKL_BOARD_LED_MODE_E mode)
{
	int ret = CMD_RET_FAILURE;
	u16 reg_val = 0, temp;
	uint32_t reg_addr = 0;
	const char *devname;

#define PHY_DEV_ADDR 		0x1F
#define PHY_LED_0_CTRL_REG 	0xF020
#define PHY_LED_1_CTRL_REG	0xF021


	ret = siklu_set_cpld_eth_led_color(led, mode);

	if (mode == SKL_LED_MODE_OFF)
		reg_val = 0;
	else if (mode == SKL_LED_MODE_GREEN_BLINK || mode == SKL_LED_MODE_YELLOW_BLINK)
		reg_val = 0x1700;
	else if (mode == SKL_LED_MODE_GREEN || mode == SKL_LED_MODE_YELLOW)
		reg_val = 0xb8;

#if defined(CONFIG_MII_INIT)
	mii_init ();
#endif

	/* use current device */
	devname = miiphy_get_current_dev();
	if (!devname) {
		printf("No available MDIO Controller!\n");
		return -1;
	}

	if (led == SKL_LED_ETH2_0)
		reg_addr = PHY_LED_0_CTRL_REG;
	else if (led == SKL_LED_ETH2_1)
		reg_addr = PHY_LED_1_CTRL_REG;

	siklu_mdio_bus_connect(SIKLU_MDIO_BUS1);

	// Phase #1 according to datasheet p93, pp3.11.1.1
	temp = (0 << 14) | (PHY_DEV_ADDR & 0x1f);
	miiphy_write(devname, PHY_88x3310_DEV_ADDR, 13, temp);
	miiphy_write(devname, PHY_88x3310_DEV_ADDR, 14, reg_addr);

	// Phase #2
	temp = (1 << 14) | (PHY_DEV_ADDR & 0x1f);
	miiphy_write(devname, PHY_88x3310_DEV_ADDR, 13, temp);
	miiphy_write(devname, PHY_88x3310_DEV_ADDR, 14, reg_val);

	return ret;
}

int siklu_set_led(SKL_BOARD_LED_TYPE_E led, SKL_BOARD_LED_MODE_E mode)
{
    int rc = CMD_RET_FAILURE;

    switch (led)
    {
    case SKL_LED_ETH1:
    	rc = siklu_set_eth1_led(mode);
    	break;
    case SKL_LED_ETH2_0:
    case SKL_LED_ETH2_1:
        rc = siklu_set_eth2_led(led, mode);
        break;
    case SKL_LED_POWER:
        rc = siklu_set_led_power(mode);
        break;
    case SKL_LED_MODEM:
    	rc = siklu_set_led_modem(mode);
    	break;
    default:
        return -1; // no handler!
        break;
    }
    return rc;
}

static int do_siklu_board_led_control(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int rc = CMD_RET_SUCCESS;
    char led[30];
    char state[30];
    SKL_BOARD_LED_MODE_E _mode;
    SKL_BOARD_LED_TYPE_E _led;

    if (argc == 3)
    {
        strcpy(led, argv[1]);
        strcpy(state, argv[2]);
    }
    else
    {
        printf("sled [led] [state]\n");
        printf(" led:   eth1/eth2_0/eth2_1/modem/power\n");
        printf(" state: \n\to - off\n\tg - green\n\ty - yellow\n\tgb - green blink\n\tyb - yellow blink\n");
        return rc;
    }


    if (strcmp(led, "eth1") == 0)
    {
        _led = SKL_LED_ETH1;
    }
    else if (strcmp(led, "eth2_0") == 0)
    {
        _led = SKL_LED_ETH2_0;
    }
    else if (strcmp(led, "eth2_1") == 0)
    {
        _led = SKL_LED_ETH2_1;
    }
    else if (strcmp(led, "modem") == 0)
    {
        _led = SKL_LED_MODEM;
    }
    else if (strcmp(led, "power") == 0)
    {
        _led = SKL_LED_POWER;
    }
    else
    {
        printf("Wrong LED type\n");
        return CMD_RET_USAGE;
    }

    if (strcmp(state, "o") == 0)
    {
        _mode = SKL_LED_MODE_OFF;
    }
    else if (strcmp(state, "g") == 0)
    {
        _mode = SKL_LED_MODE_GREEN;
    }
    else if (strcmp(state, "y") == 0)
    {
        _mode = SKL_LED_MODE_YELLOW;
    }
    else if (strcmp(state, "gb") == 0)
    {
        _mode = SKL_LED_MODE_GREEN_BLINK;
    }
    else if (strcmp(state, "yb") == 0)
    {
        _mode = SKL_LED_MODE_YELLOW_BLINK;
    }
    else
    {
        printf("Wrong LED mode\n");
        return CMD_RET_USAGE;
    }

    rc = siklu_set_led(_led, _mode);
    if (rc != 0)
    {
        printf(" Error or unsupported mode\n");
        rc = CMD_RET_SUCCESS; // return success here
    }

    return rc;
}



U_BOOT_CMD(sled, 3, 1, do_siklu_board_led_control, "Control Onboard LEDs", "[led] [state] Control Onboard LEDs");
