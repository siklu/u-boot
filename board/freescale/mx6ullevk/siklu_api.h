/*
 * siklu_api.h
 *
 *  Created on: Aug 29, 2017
 *      Author: edwardk
 */

#ifndef SIKLU_API_H_
#define SIKLU_API_H_


extern int siklu_is_restore2fact_default(void);
extern const char* siklu_mutable_env_get(const char *varname);


extern uint32_t get_nand_part_offset_by_name(const char* name);

#endif /* SIKLU_API_H_ */
