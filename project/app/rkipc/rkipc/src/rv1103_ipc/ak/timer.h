/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   12/01/2017
 * @brief: ak timer
 ******************************************************************************
 **/
#ifndef __TIMER_H__
#define __TIMER_H__

#include "message.h"

extern void *timer_entry(void *);
extern q_msg_t timer_mailbox;

#endif	  //__TIMER_H__
