
#include <common.h>

// read gpio pin by number
int siklu_read_gpio_by_number(unsigned int gpio_num, int *val)
{
	
	int ret = CMD_RET_SUCCESS;

	// grab the pin before we tweak it
	ret = gpio_request(gpio_num, "cmd_gpio");
	if (ret != CMD_RET_SUCCESS) {
		return ret;
	}

	// read the gpio value
	gpio_direction_input(gpio_num);
	*val = gpio_get_value(gpio_num);

	// free pin
	gpio_free(gpio_num);

	return CMD_RET_SUCCESS;
}


// read gpio pin by name
int siklu_read_gpio_by_name(const char * str_gpio, int *val)
{

	int ret = CMD_RET_SUCCESS;
	int gpio_num = 0;

	ret = gpio_lookup_name(str_gpio, NULL, NULL, &gpio_num);
	if (ret != CMD_RET_SUCCESS) {
		return ret;
	}

	ret = siklu_read_gpio_by_number(gpio_num, val);
	if (ret != CMD_RET_SUCCESS) {
		return ret;
	}

	return CMD_RET_SUCCESS;
}
