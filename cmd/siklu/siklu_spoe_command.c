#include <common.h>
#include <command.h>
#include <dm/device.h>

#include <siklu/cmd/siklu_spoe_command.h>

#include <asm/gpio.h>

const char N366_SPOE_NAME[] = "cpm_gpio17";
const char TG_TU_SPOE_NAME[] = "gpio@18_0";

enum PoE_PAIRS siklu_poe_num_pairs()
{
	enum PoE_PAIRS ret = UNKNOWN_PAIRS;
	unsigned gpio_no;
	const char * failure = NULL;
	int pairs;
	const char * spoe_name = NULL;

	if(of_machine_is_compatible("siklu,n366"))
	{
		spoe_name = N366_SPOE_NAME;
	}
	else if (of_machine_is_compatible("siklu,tg-tu"))
	{
		spoe_name = TG_TU_SPOE_NAME;
	}
	else
	{
		failure = "non-suported board";
		goto error;
	}

	if(gpio_lookup_name(spoe_name, NULL, NULL, &gpio_no))
	{
		failure = "gpio_lookup_name";
		goto error;
	}

	if(gpio_request(gpio_no, "spoe"))
	{
		failure = "gpio_request";
		goto error;
	}

	if(gpio_direction_input(gpio_no))
	{
		failure = "gpio_direction_input";
		goto error_free_gpio;
	}

	pairs = gpio_get_value(gpio_no);

	if (pairs == 0)
		ret = TWO_PAIRS;
	else if(pairs == 1)
		ret = FOUR_PAIRS;
	else /*Expected -1 */
	{
		failure = "gpio_get_value";
		goto error_free_gpio;
	}

error_free_gpio:
	gpio_free(gpio_no);
error:
	if(ret == UNKNOWN_PAIRS)
		printf("Failed on %s", failure);
	return ret;
}

static int do_siklu_poe_num_pairs_show(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	enum PoE_PAIRS pairs = siklu_poe_num_pairs();

	int ret = CMD_RET_FAILURE;
	if (pairs == TWO_PAIRS)
	{
		printf("2-pairs\n");
		ret = CMD_RET_SUCCESS;
	}
	else if(pairs == FOUR_PAIRS)
	{
		printf("4-pairs\n");
		ret = CMD_RET_SUCCESS;
	}
	else
		printf("Unknown\n");
	return ret;
}

U_BOOT_CMD(spoe, 5, 0, do_siklu_poe_num_pairs_show,
	"Show POE number pairs Status", "Show POE number pairs Status");
