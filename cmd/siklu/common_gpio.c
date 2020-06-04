#include <common.h>
#include <asm/gpio.h>

// read gpio pin by number
int siklu_read_gpio_by_number(unsigned int gpio_num, u32 *val)
{
	int ret = 0;

	// request the gpio pin
	ret = gpio_request(gpio_num, "siklu_common_gpio");
	if (ret) 
		goto out;

	// read the gpio value
	ret = gpio_direction_input(gpio_num);
	if (ret)
		goto out_free;

	*val = gpio_get_value(gpio_num);

out_free:
	gpio_free(gpio_num);
out:
	return ret;
}


// read gpio pin by name
int siklu_read_gpio_by_name(const char * str_gpio, u32 *val)
{
	int ret = 0;
	unsigned int gpio_num = 0;

	ret = gpio_lookup_name(str_gpio, NULL, NULL, &gpio_num);
	if (ret) {
		return ret;
	}

	ret = siklu_read_gpio_by_number(gpio_num, val);
	if (ret) {
		return ret;
	}

	return 0;
}