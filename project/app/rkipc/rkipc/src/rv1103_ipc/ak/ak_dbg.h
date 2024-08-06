/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   12/01/2017
 * @brief: debug ak function
 ******************************************************************************
 **/
#ifndef __AK_DBG_H__
#define __AK_DBG_H__

#include <stdio.h>
#include "sys_dbg.h"

#define AK_MSG_DBG_EN	0
#define AK_TIMER_DBG_EN 0
#define AK_TIMER_WRN_EN 1
#define AK_PRINT_EN		1

#if (AK_MSG_DBG_EN == 1)
#define AK_MSG_DBG(fmt, ...) __LOG__(fmt, "AK_MSG", ##__VA_ARGS__)
#else
#define AK_MSG_DBG(fmt, ...)
#endif

#if (AK_TIMER_DBG_EN == 1)
#define AK_TIMER_DBG(fmt, ...) __LOG__(fmt, "AK_TIMER", ##__VA_ARGS__)
#else
#define AK_TIMER_DBG(fmt, ...)
#endif

#if (AK_TIMER_WRN_EN == 1)
#define AK_TIMER_WRN(fmt, ...) __LOG__(fmt, "AK_TIMER_WRN", ##__VA_ARGS__)
#else
#define AK_TIMER_WRN(fmt, ...)
#endif

#if (AK_PRINT_EN == 1)
#define AK_PRINT(fmt, ...) __LOG__(fmt, "AK_PRINT", ##__VA_ARGS__)
#else
#define AK_PRINT(fmt, ...)
#endif

#endif	  //__AK_DBG_H__
