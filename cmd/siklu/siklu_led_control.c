/* Copyright 2020 by Siklu LTD. All rights reserved. */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <led.h>
#include <dm/uclass-internal.h>
#include <../net/eth_internal.h>
#include <siklu/common_gpio.h>

typedef enum
{
	SIKLU_LED_POWER,
	SIKLU_LED_WLAN,
	SIKLU_LED_ETH1,
	SIKLU_LED_ETH2,
	SIKLU_LED_ETH3,
	SIKLU_LED_MAX
} SIKLU_LED_TYPE;

typedef enum
{
	SIKLU_YELLOW_COLOR,
	SIKLU_GREEN_COLOR,
	SIKLU_COLOR_MAX
} SIKLU_LED_COLOR;

typedef struct
{
	const char *yellow_label;
	const char *green_label;
	void(*board_hook)(SIKLU_LED_COLOR color, enum led_state_t state);
} siklu_dual_led_def_t;

static const char* const siklu_led_label_to_str[SIKLU_LED_MAX] =
{
	[SIKLU_LED_POWER]	= "power",
	[SIKLU_LED_WLAN]	= "wlan",
	[SIKLU_LED_ETH1]	= "eth1",
	[SIKLU_LED_ETH2]	= "eth2",
	[SIKLU_LED_ETH3]	= "eth3",
};

static void n366_eth1_led_hook(SIKLU_LED_COLOR color, enum led_state_t state){
	if(color == SIKLU_YELLOW_COLOR) {
		siklu_write_gpio_by_name("cpm_gpio122", 0);
	} else {
		siklu_write_gpio_by_name("cpm_gpio122", 1);
	}
}

static int siklu_led_set_state(const siklu_dual_led_def_t *def, SIKLU_LED_COLOR color, enum led_state_t state) {
	int ret;
	const char *led_label;
	struct udevice *dev;

	if(def->board_hook) {
		def->board_hook(color, state);
	}

	led_label = (color == SIKLU_GREEN_COLOR ? def->green_label : def->yellow_label);

	ret = led_get_by_label(led_label, &dev);
	if (ret) 
		return ret;

	ret = led_set_state(dev, state);
	if (ret)
		return ret;

	return ret;
}

static const siklu_dual_led_def_t siklu_led_label_to_n366[SIKLU_LED_MAX] =
{
	[SIKLU_LED_POWER] = {
			.yellow_label = "power-y",
			.green_label  = "power-g",
	},
	[SIKLU_LED_WLAN] = {
			.yellow_label	= "modem-y",
			.green_label 	= "modem-g",
	},
	[SIKLU_LED_ETH2] = {
			.yellow_label	= "cps-mdio-0-led2",
			.green_label	= "cps-mdio-0-led2",
			.board_hook = n366_eth1_led_hook,
	},
	[SIKLU_LED_ETH1] = {
			.yellow_label	= "cpm-xmdio-2-led2",
			.green_label	= "cpm-xmdio-2-led1",
	},
	[SIKLU_LED_ETH3] = {
			.yellow_label	= "sfp-y",
			.green_label	= "sfp-g",
	},
};

static const siklu_dual_led_def_t siklu_led_label_to_tg_tu[SIKLU_LED_MAX] =	//TO DO
{
	[SIKLU_LED_POWER] = {
			.yellow_label	= NULL,
			.green_label	= NULL,
	},
	[SIKLU_LED_WLAN] = {
			.yellow_label	= NULL,
			.green_label 	= NULL,
	},
	[SIKLU_LED_ETH1] = {
			.yellow_label	= NULL,
			.green_label	= NULL,
	},
	[SIKLU_LED_ETH2] = {
			.yellow_label	= NULL,
			.green_label	= NULL,
	},
	[SIKLU_LED_ETH3] = {
			.yellow_label	= NULL,
			.green_label	= NULL,
	},
};

static const siklu_dual_led_def_t* led_arr_get(void)
{
	if(of_machine_is_compatible("siklu,n366"))
		return siklu_led_label_to_n366;
	else if (of_machine_is_compatible("siklu,tg-tu"))
		return siklu_led_label_to_tg_tu;
	else
		return NULL;
}

/*
 * gets a string of sled name and returns its position in the sled array
 */
static SIKLU_LED_TYPE sled_str_to_led_type(const char *sled)
{
	for (int i = 0; i < SIKLU_LED_MAX; i++)
	{
		if (strncmp(sled, siklu_led_label_to_str[i], strlen(sled)) == 0)
			return i;
	}
	return SIKLU_LED_MAX;
}

static enum led_state_t sled_status_to_led_status(const char* status)
{
	if (strcmp(status, "o") == 0)
		return LEDST_OFF;
	else
		return LEDST_ON;
}

static int sled_color_to_sled_pos(const char* color)
{
	if (strcmp(color, "y") == 0)
		return SIKLU_YELLOW_COLOR;
	else if (strcmp(color, "g") == 0)
		return SIKLU_GREEN_COLOR;
	else
		return SIKLU_COLOR_MAX;
}

/*
 * we need to initialize the mdio bus in order for the phy driver
 * to load and initialize its LED API. This function does that.
 */
static int load_ethernet_device(void)
{
	static int ethernet_loaded = 0;
	struct udevice *current_device = eth_get_dev();
	// save global data flags
	unsigned long save_flags = gd->flags;

	if (ethernet_loaded == 1)
		return 0;


	// disable console to a ignore internal prints of nand_get_flash_type
	gd->flags |= (GD_FLG_SILENT | GD_FLG_DISABLE_CONSOLE);

	do
	{
		eth_set_current_to_next();

		eth_halt();
		eth_init();
	} while(eth_get_dev() != current_device);

	ethernet_loaded = 1;

	// restore previous flags
	gd->flags = save_flags;

	return CMD_RET_SUCCESS;
}

/*
 * takes care of ALL command in siklu u-boot
 */
static int all_leds_color(SIKLU_LED_COLOR led_color)
{
	const siklu_dual_led_def_t* led_arr = led_arr_get();
	int i;

	for(i = 0; i < SIKLU_LED_MAX; ++i) {
		siklu_led_set_state(&led_arr[i], led_color, LEDST_ON);
	}
	return 0;
}

static int turn_off_all_leds() {
		const siklu_dual_led_def_t* led_arr = led_arr_get();
	int i;
	int color;

	for(i = 0; i < SIKLU_LED_MAX; ++i) {
		for(color = 0; color < SIKLU_COLOR_MAX; ++color) {
			siklu_led_set_state(&led_arr[i], color, LEDST_OFF);
		}
	}
	return 0;
}

static int do_led_control(cmd_tbl_t *cmdtp, int flag, int argc,
					  char *const argv[])
{
	enum led_state_t cmd;
	const siklu_dual_led_def_t* led_arr = led_arr_get();

	int ret, led_color;
	int i;
	SIKLU_LED_TYPE led_type;

	/* Validate arguments */
	if (argc != 3)
		return CMD_RET_USAGE;

	ret = load_ethernet_device();
	if (ret < 0) {
		printf("LED mdio didn't load properly (err=%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	cmd = sled_status_to_led_status(argv[2]);
	led_color = sled_color_to_sled_pos(argv[2]);
	if(led_color == SIKLU_COLOR_MAX && cmd != LEDST_OFF) {
		printf("Invalid led status: %s", argv[2]);
		return CMD_RET_USAGE;
	}

	if (strcmp(argv[1], "all") == 0) {		
		if(cmd == LEDST_ON) {
			all_leds_color(led_color);
		} else {
			turn_off_all_leds();
		}

		return 0;
	}

	led_type = sled_str_to_led_type(argv[1]);
	if(led_type == SIKLU_LED_MAX) {
		printf("Invalid led: %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}
	
	ret = siklu_led_set_state(&led_arr[led_type], led_color, cmd);
	if(ret) {
		printf("Failed to set led: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
		sled,
		3,
		0,
		do_led_control,
		"siklu led control",
		"Allows you to control the power, wlan, eth1, eth2 and eth3 leds\n sled < power | wlan | eth1 | eth2 | eth3 | all > < o | g | y >"
);

