
#include <common.h>

/**
 * read gpio specific pin by number (0-159)
 * @param gpio - 0-159 a pointer.
 * @return <0 on error, 0 otherwise. 
 */
int siklu_read_gpio(unsigned int gpio, int *val)
{
	int ret = CMD_RET_SUCCESS;

	// grab the pin before we tweak it
	ret = gpio_request(gpio, "cmd_gpio");
	if (ret != CMD_RET_SUCCESS) {
		return ret;
	}

	// read the gpio value
	gpio_direction_input(gpio);
	*val = gpio_get_value(gpio);

	// free pin
	gpio_free(gpio);

	return CMD_RET_SUCCESS;
}
