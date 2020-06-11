/* Copyright 2020 by Siklu LTD. All rights reserved. */

#include <common.h>
#include <command.h>
#include <dm.h>
#include <led.h>
#include <dm/uclass-internal.h>
#include <../net/eth_internal.h>

typedef struct
{
	const char *yellow_label;
	const char *green_label;
} siklu_dual_led_def_t;

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

static const char* const siklu_led_label_to_str[SIKLU_LED_MAX] =
{
	[SIKLU_LED_POWER]	= "power",
	[SIKLU_LED_WLAN]	= "wlan",
	[SIKLU_LED_ETH1]	= "eth1",
	[SIKLU_LED_ETH2]	= "eth2",
	[SIKLU_LED_ETH3]	= "eth3",
};

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
	[SIKLU_LED_ETH1] = {
			.yellow_label	= "cps-mdio-0-led1",
			.green_label	= "cps-mdio-0-led2",
	},
	[SIKLU_LED_ETH2] = {
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
 * gets a string of led name and returns if it's a yellow or green led
 */
static int led_str_to_color_pos(const char* led, const siklu_dual_led_def_t* led_arr)
{
	for (int i = 0; i < SIKLU_LED_MAX; i++)
	{
		if (strncmp(led, led_arr[i].yellow_label, strlen(led)) == 0)
			return SIKLU_YELLOW_COLOR;
		else if (strncmp(led, led_arr[i].green_label, strlen(led)) == 0)
			return SIKLU_GREEN_COLOR;
	}
	return SIKLU_COLOR_MAX;
}

/*
 * gets a string of sled name and returns its position in the sled array
 */
static int sled_str_to_sled_pos(const char *sled)
{
	for (int i = 0; i < SIKLU_LED_MAX; i++)
	{
		if (strncmp(sled, siklu_led_label_to_str[i], strlen(sled)) == 0)
			return i;
	}
	return SIKLU_LED_MAX;
}

/*
 * gets a string of sled name and led color and returns its position in the led array
 */
static const char* sled_str_to_led_str (const char* sled, SIKLU_LED_COLOR led_color)
{
	int led_pos = sled_str_to_sled_pos(sled);
	const siklu_dual_led_def_t* led_arr = led_arr_get();

	return (led_color == SIKLU_YELLOW_COLOR) ? led_arr[led_pos].yellow_label : led_arr[led_pos].green_label;
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
static int all_leds(enum led_state_t cmd_for_all, SIKLU_LED_COLOR led_color)
{
	struct udevice *dev;
	int ret;
	const siklu_dual_led_def_t* led_arr = led_arr_get();

	for (uclass_find_first_device(UCLASS_LED, &dev);
		 dev;
		 uclass_find_next_device(&dev))
	{
		struct led_uc_plat *plat = dev_get_uclass_platdata(dev);
		if (!plat->label)
			continue;

		if ((cmd_for_all == LEDST_ON) && (led_str_to_color_pos(plat->label, led_arr) != led_color))
			continue;

		ret = led_get_by_label(plat->label, &dev);
		if (ret) {
			printf("LED '%s' not found (err=%d)\n", plat->label, ret);
			return CMD_RET_FAILURE;
		}

		switch (cmd_for_all)
		{
			case LEDST_OFF:
			case LEDST_ON:
				ret = led_set_state(dev, cmd_for_all);
				break;
			default:
				ret = -1;
				break;
		}
		if (ret < 0) {
			printf("LED '%s' operation failed (err=%d)\n", plat->label, ret);
			return CMD_RET_FAILURE;
		}
	}
	return 0;
}

static int do_led_control(cmd_tbl_t *cmdtp, int flag, int argc,
					  char *const argv[])
{
	enum led_state_t cmd;
	const char* led_label;
	struct udevice *dev;

	int ret, led_color;

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

	if (strcmp(argv[1], "all") == 0)
		return all_leds(cmd, led_color);

	for (int i = 0; i < SIKLU_COLOR_MAX; i++)
	{
		if (cmd == LEDST_OFF)
			led_label = sled_str_to_led_str(argv[1], i);
		else
			led_label = sled_str_to_led_str(argv[1], led_color);

		ret = led_get_by_label(led_label, &dev);
		if (ret) {
			printf("LED '%s' not found (err=%d)\n", argv[1], ret);
			return CMD_RET_FAILURE;
		}

		ret = led_set_state(dev, cmd);
		if (ret < 0) {
			printf("LED '%s' operation failed (err=%d)\n", argv[1], ret);
			return CMD_RET_FAILURE;
		}

		if (cmd == LEDST_ON)
			break;
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

