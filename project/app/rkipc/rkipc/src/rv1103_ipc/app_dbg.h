#ifndef __APP_DBG_H__
#define __APP_DBG_H__

#include <stdio.h>
#include "sys_dbg.h"

#define APP_DBG_EN		  1
#define APP_PRINT_EN	  1
#define APP_ERR_EN		  0
#define APP_DBG_SIG_EN	  1
#define APP_DBG_DRIVER_EN 0
#define APP_LOG_EN		  1

/* module debug */
#define SD_DEBUG	 0
#define RECORD_DEBUG 0

#if (APP_PRINT_EN == 1)
#define APP_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define APP_PRINT(fmt, ...)
#endif

#if !defined(RELEASE) && (APP_DBG_EN == 1)
#define APP_DBG(fmt, ...) __LOG__(fmt, "APP_DBG", ##__VA_ARGS__)
#else
#define APP_DBG(fmt, ...)
#endif

#if !defined(RELEASE) && (APP_DBG_DRIVER_EN == 1)
#define APP_DRIVER(fmt, ...) __LOG__(fmt, "APP_DRIVER", ##__VA_ARGS__)
#else
#define APP_DRIVER(fmt, ...)
#endif

#if (APP_ERR_EN == 1)
#define APP_ERR(x...)                                         \
	do {                                                      \
		printf("\033[1;31m%s->%d: ", __FUNCTION__, __LINE__); \
		printf(x);                                            \
		printf("\033[0m\n");                                  \
	} while (0)
#else
#define APP_ERR(fmt, ...)
#endif

#if (APP_DBG_SIG_EN == 1)
#define APP_DBG_SIG(fmt, ...) __LOG__(fmt, "SIG -> ", ##__VA_ARGS__)
#else
#define APP_DBG_SIG(fmt, ...)
#endif

#if (APP_LOG_EN == 1)
#define APP_LOG_INFO(fmt, ...)		__LOG__(fmt, "INFO", ##__VA_ARGS__)
#define APP_LOG_WARNING(fmt, ...) 	__LOG__(fmt, "WARNING", ##__VA_ARGS__)
#define APP_LOG_ERROR(fmt, ...)		__LOG__(fmt, "ERROR", ##__VA_ARGS__)
#define APP_LOG_FATAL(fmt, ...)		__LOG__(fmt, "FATAL", ##__VA_ARGS__)
#else
#define APP_LOG_INFO	PLOG_NONE_(PLOG_APP_INSTANCE_ID)
#define APP_LOG_WARNING PLOG_NONE_(PLOG_APP_INSTANCE_ID)
#define APP_LOG_ERROR	PLOG_NONE_(PLOG_APP_INSTANCE_ID)
#define APP_LOG_FATAL	PLOG_NONE_(PLOG_APP_INSTANCE_ID)
#endif

#endif	  //__APP_DBG_H__
