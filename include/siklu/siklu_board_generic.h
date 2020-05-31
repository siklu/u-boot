#ifndef __SIKLU_BOARD_GENERIC_H
#define __SIKLU_BOARD_GENERIC_H

#define ILLEGAL_HW_REVISION -1

/**
 * Get the board HW revision
 * @param out: HW revision 
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
siklu_get_hw_revision(int *hw_revsion);



/**
 * get cpu name. 
 * @param out: cpu_name 
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
siklu_get_cpu_name (const char **cpu_name);


/**
 * get cpu config register 
 * This is a register that holds general config attributes of the CPU (e.g, freq mode). it is needed for BIST
 * @param out: config_reg 
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
siklu_get_cpu_config_register(uint64_t *config_reg);

#endif
