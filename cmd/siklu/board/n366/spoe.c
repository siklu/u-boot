//#include <common.h>
//#include <command.h>
#include <asm/gpio.h>

#include "../../siklu_spoe_command.h"

const char N366_SPOE_NAME[] = "cpm_gpio18";

enum PoE_PAIRS poe_num_pairs()
{
	enum PoE_PAIRS ret = UNKNOWN_PAIRS;
	unsigned gpio_no;
	const char * failure = NULL;
	int pairs;
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

