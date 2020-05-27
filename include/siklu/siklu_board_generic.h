#ifndef __SIKLU_BOARD_GENERIC_H
#define __SIKLU_BOARD_GENERIC_H

#define ILLEGAL_HW_REVISION -1

/**
 * Get the save HW revision as it was calculated before the board init
 * @return the saved HW revision.
 */
int 
siklu_get_saved_hw_revision(void);


/**
 * calculate and save siklu hw revision. 
 * find the specific board, call the specific board HW revision calculation and save it in a static var
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
siklu_calculate_and_save_siklu_hw_revision (void);


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
