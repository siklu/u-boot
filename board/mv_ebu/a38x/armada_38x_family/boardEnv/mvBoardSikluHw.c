/*
 * mvBoardSikluHw.c
 *
 *  Created on: Jun 9, 2016
 *      Author: edwardk
 */

#include <common.h>
#include <command.h>
#include "mvCommon.h"
#include "mvOs.h"
#include <siklu_api.h>

#define	MV_GPP_IN	0xFFFFFFFF	/* GPP input */
#define MV_GPP_OUT	0		/* GPP output */

extern MV_STATUS mvGppValueSet(MV_U32 group, MV_U32 mask, MV_U32 value);
extern MV_STATUS mvGppTypeSet(MV_U32 group, MV_U32 mask, MV_U32 value);
extern MV_U32 mvGppValueGet(MV_U32 group, MV_U32 mask);
/*
 *
 */
int mvSikluGpioSetVal(int gppNum, int val) {
	int rc = 0;

	if (gppNum <= 31) {
		val = !!val;
		mvGppTypeSet(0, 1 << gppNum, (MV_GPP_OUT & (1 << gppNum))); // configure gppX as output
		mvGppValueSet(0, 1 << gppNum, val & (1 << gppNum));
	}

	return rc;

}
/*
 *
 */
int mvSikluGpioSetDirection(int gppNum, int isOutput) {
	int rc = 0;
	if (gppNum >= 32)
		return -1;

	if (isOutput) {
		mvGppTypeSet(0, 1 << gppNum, (MV_GPP_OUT & (1 << gppNum))); // configure gppX as output
	} else {
		mvGppTypeSet(0, 1 << gppNum, (MV_GPP_IN & (1 << gppNum))); // configure gppX as input
	}

	return rc;

}
/*
 *
 */
int mvSikluGpioGetVal(int gppNum, int* val) {
	int rc = 0;
	if (gppNum >= 32)
		return -1;
	*val = !! mvGppValueGet(0, 1 << gppNum);
	return rc;
}

