#ifndef __SIKLU_GPIO_H__
#define __SIKLU_GPIO_H__

/**
 * read gpio specific pin by number (0-159)
 * @param gpio - 0-159 a pointer.
 * @return <0 on error, 0 otherwise. 
 */
int siklu_read_gpio(unsigned int gpio, int *val);

#endif
