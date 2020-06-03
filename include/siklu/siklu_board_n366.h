#ifndef __SIKLU_BOARD_N366_H
#define __SIKLU_BOARD_N366_H


/**
 * Get N366 HW revision. This is a board specific function.
 * @param out: hw_revision 
 * @return <0 on error, 0 otherwise. 
 */
int 
siklu_n366_get_hw_revision (u32 *hw_revision);


/**
 * get cpu name (specific for n366 board) 
 * @param out: cpu_name 
 * @return <0 on error, 0 otherwise. 
 */
int 
siklu_n366_get_cpu_config_register(uint64_t *config_reg);


/**
 * get cpu config register (specific for n366 board) 
 * This is a register that holds general config attributes of the CPU (e.g, freq mode). it is needed for BIST
 * @param out: config_reg 
 * @return <0 on error, 0 otherwise. 
 */
int 
siklu_n366_get_cpu_name(const char **cpu_name);



#endif
