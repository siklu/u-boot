#ifndef __SIKLU_BOARD_HW_REVISION_H
#define __SIKLU_BOARD_HW_REVISION_H

int siklu_get_hw_revision (unsigned int *hw_revision);

/**
 * Get HW revision. This is a board specific function.
 * @param out: hw_revision 
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int 
siklu_get_hw_revision (unsigned int *hw_revision);


#endif
