#include <common.h>
#include <command.h>

#include <siklu/cmd/siklu_spoe_command.h>
#include <siklu_load_device_configurations.h>

#include <asm/gpio.h>

const char N366_SPOE_NAME[] = "cpm_gpio17";

enum PoE_PAIRS siklu_poe_num_pairs()
{
	enum PoE_PAIRS ret = UNKNOWN_PAIRS;
	unsigned gpio_no;
	const char * failure = NULL;
	int pairs;
	if(!of_machine_is_compatible("siklu,n366"))
	{
		failure = "non N366 board";
		goto error;
	}
	if(gpio_lookup_name(N366_SPOE_NAME, NULL, NULL, &gpio_no))
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

	if (pairs == TWO_PAIRS)
		printf("2-pairs\n");
	else if(pairs == FOUR_PAIRS)
		printf("4-pairs\n");
	else if(pairs == UNKNOWN_PAIRS)
		printf("Unknown\n");

	return pairs != UNKNOWN_PAIRS ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

U_BOOT_CMD(spoe, 5, 0, do_siklu_poe_num_pairs_show,
	"Show POE number pairs Status", "Show POE number pairs Status");
