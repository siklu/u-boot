/*
 * siklu_api.h
 *
 *  Created on: Jun 9, 2016
 *      Author: edwardk
 */

#ifndef INCLUDE_SIKLU_API_H_
#define INCLUDE_SIKLU_API_H_






extern int mvSikluGpioSetVal(int gpio, int val);
extern int mvSikluGpioSetDirection(int gpio, int isOutput);
extern int mvSikluGpioGetVal(int gpio, int* val);


#endif /* INCLUDE_SIKLU_API_H_ */
