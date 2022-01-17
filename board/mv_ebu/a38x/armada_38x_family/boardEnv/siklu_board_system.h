/*
 * siklu_board_system.h
 *
 *  Created on: Nov 16, 2016
 *      Author: edwardk
 */

#ifndef SIKLU_BOARD_SYSTEM_H_
#define SIKLU_BOARD_SYSTEM_H_

typedef enum
{
    SIKLU_NETWORK_PORT_TYPE_NONE,  // port doesn't installed
    SIKLU_NETWORK_PORT_TYPE_COPPER, // copper port with PHY
    SIKLU_NETWORK_PORT_TYPE_FIBER, // in-band connection without PHY, SFP connected to MAC
} SIKLU_NETWORK_PORT_TYPE_E;


SIKLU_NETWORK_PORT_TYPE_E siklu_get_network_port_type(int port_num);




#endif /* SIKLU_BOARD_SYSTEM_H_ */
