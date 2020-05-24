#ifndef __SIKLU_DEVICE_SPECIFIC_INFORMATION_H
#define __SIKLU_DEVICE_SPECIFIC_INFORMATION_H


/**
 * Get CPU config register. Control register which is specific for the exist CPU (needed for beast)
 * @param out: config_reg 
 * @return FDT blob, on success; NULL on error.
 */
int 
siklu_get_cpu_config_register(int *config_reg);

#endif




