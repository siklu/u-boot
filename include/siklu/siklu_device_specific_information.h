#ifndef __SIKLU_DEVICE_SPECIFIC_INFORMATION_H
#define __SIKLU_DEVICE_SPECIFIC_INFORMATION_H


#define MAX_CPU_NAME_STR 100

/**
 * Get CPU config register. Control register which is specific for the exist CPU (needed for beast)
 * @param out: config_reg 
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
siklu_get_cpu_config_register(int *config_reg);


/**
 * Get CPU name 
 * @param out: cpu_name (string)
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
siklu_get_cpu_name(const char **cpu_name);


#endif




