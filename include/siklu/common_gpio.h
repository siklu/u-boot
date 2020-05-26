#ifndef __SIKLU_GPIO_H__
#define __SIKLU_GPIO_H__

/**
 * read gpio specific pin by number (0-159)
 * This code is mostly copied from cmd/gpio.c 
 * @param gpio_num - 0-159 
 * @return <0 on error, 0 otherwise. 
 */
int siklu_read_gpio_by_number(unsigned int gpio_num, int *val);


/**
 * read gpio specific pin by name 
 * @param gpio - gpio name (e.g, "cpm_gpio126")
 * @return <0 on error, 0 otherwise. 
 */
int siklu_read_gpio_by_name(const char * str_gpio, int *val);

#endif
