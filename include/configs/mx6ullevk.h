/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6UL 14x14 EVK board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 *  * The file is selector only
 */
#ifndef __MX6ULLEVK_CONFIG_H_
#define __MX6ULLEVK_CONFIG_H_


	#ifdef CONFIG_SIKLU_BOARD
	# include <configs/mx6ullevk_siklu.h>
	#else
	# include <configs/mx6ullevk_evk.h>
	#endif /* CONFIG_SIKLU_BOARD  */

#endif


