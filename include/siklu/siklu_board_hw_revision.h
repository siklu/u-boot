#ifndef __SIKLU_BOARD_HW_REVISION_H
#define __SIKLU_BOARD_HW_REVISION_H

/**
 * Get the save HW revision as it was calculated before th board init
 * @param out: saved_hw_revision 
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
siklu_get_saved_hw_revision(int *saved_hw_revision);


/**
 * calculate and save siklu hw revision. 
 * find the specific board, call the specific board HW revision calculation and save in a static var
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
calculate_and_save_siklu_hw_revision (void);


#endif
