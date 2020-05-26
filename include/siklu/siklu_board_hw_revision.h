#ifndef __SIKLU_BOARD_HW_REVISION_H
#define __SIKLU_BOARD_HW_REVISION_H


/**
 * Get N366 HW revision. This is a board specific function.
 * @param out: hw_revision 
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int siklu_n366_get_hw_revision (unsigned int *hw_revision);


/**
 * Get the save HW revision as it was calculated before th board init
 * @param out: saved_hw_revision 
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
siklu_get_saved_hw_revision(int *saved_hw_revision);


#endif
