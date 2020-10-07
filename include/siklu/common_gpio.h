#ifndef __SIKLU_GPIO_H__
#define __SIKLU_GPIO_H__

/**
 * read gpio specific pin by number (0-159)
 * This code is mostly copied from cmd/gpio.c 
 * @param gpio_num - 0-159 
 * @return <0 on error, 0 otherwise. 
 */
int siklu_read_gpio_by_number(unsigned int gpio_num, u32 *val);


/**
 * read gpio specific pin by name 
 * @param gpio - gpio name (e.g, "cpm_gpio126")
 * @return <0 on error, 0 otherwise. 
 */
int siklu_read_gpio_by_name(const char * str_gpio, u32 *val);

/**
 * @brief Write to a specific gpio by name.
 * @param str_gpio gpio - gpio name (e.g, "cpm_gpio126")
 * @param val (1 or 0)
 * @return <0 on error, 0 otherwise. 
 */
int siklu_write_gpio_by_name(const char * str_gpio, u32 val);

#endif
