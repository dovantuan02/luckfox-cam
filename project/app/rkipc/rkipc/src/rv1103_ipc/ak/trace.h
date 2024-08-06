/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   17/01/2018
 * @brief: ak trace
 ******************************************************************************
 **/
#ifndef __TRACE_H__
#define __TRACE_H__

#include <stdint.h>
#include <stdbool.h>
#include "message.h"

#define RET_TRACE_OK					0x00
#define RET_TRACE_ERR_INIT_SOCKET		0x01
#define RET_TRACE_ERR_INIT_BIND			0x02
#define RET_TRACE_ERR_INIT_LISTEN		0x03
#define RET_TRACE_ERR_INIT_HOST_SOCKET	0x04
#define RET_TRACE_ERR_INIT_HOST_CONNECT 0x05

extern uint8_t trace_msg_init();
extern void trace_msg_put(ak_msg_t *, uint32_t, time_t);
extern void trace_msg_get(ak_msg_t *, uint32_t, time_t);
extern void trace_msg_free(ak_msg_t *, uint32_t, time_t);

#endif	  //__TRACE_H__
