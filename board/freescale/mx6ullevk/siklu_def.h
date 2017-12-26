/*
 * siklu_def.h
 *
 *  Created on: Aug 29, 2017
 *      Author: edwardk
 */

#ifndef SIKLU_DEF_H_
#define SIKLU_DEF_H_


/* no need here, declared in include/configs/mx6ullevk_siklu.h
#define MTDPARTS_DEFAULT   "mtdparts=gpmi-nand:128k(env_ro),128k(env_var0),128k(env_var1),128k(hdr0),40M(uimage0),"  \
    "128k(hdr1),40M(uimage1)," "16M(conf),-(log)"   // edikk TBD
*/

#define SIKLU_BIST_ENVIRONMENT_NAME "bist"
#define SIKLU_BIST_MUT_ENVIRONMENT_NAME "SK_bist" // siklu_remark002



#define CONFIG_CPLD_DEFAULT_BUS		1
#define CONFIG_CPLD_DEFAULT_CS		1
#define CONFIG_CPLD_DEFAULT_SPEED	10000000
#define CONFIG_CPLD_DEFAULT_MODE	SPI_MODE_0


#endif /* SIKLU_DEF_H_ */
