#ifndef __SIKLU_N366_BOARD_HW_REVISION_H
#define __SIKLU_N366_BOARD_HW_REVISION_H


/**
 * Get N366 HW revision. This is a board specific function.
 * @param out: hw_revision 
 * @return CMD_RET_SUCCESS on success, otherwise on error.
 */
int siklu_n366_get_hw_revision (unsigned int *hw_revision);


#endif
