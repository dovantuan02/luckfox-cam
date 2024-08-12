#ifndef __APP_H__
#define __APP_H__

#include <string>
#include "ak.h"


using namespace std;

/*****************************************************************************/
/* task GW_SYS define.
 */
/*****************************************************************************/
/* define timer */
#define GW_SYS_WATCH_DOG_TIMEOUT_INTERVAL		(120)	  // 120s watchdog
#define GW_SYS_WATCH_DOG_PING_TASK_REQ_INTERVAL (5000)	  // 5s reset watchdog
/* define signal */
enum {
	GW_SYS_WATCH_DOG_PING_NEXT_TASK_REQ = AK_USER_DEFINE_SIG,
	GW_SYS_WATCH_DOG_PING_NEXT_TASK_RES,
	GW_SYS_RESET_WATCH_DOG_REQ,
	GW_SYS_CLOUD_REBOOT_REQ,
	GW_SYS_REBOOT_REQ,
	GW_SYS_CMD_REBOOT_REQ,
	GW_SYS_START_WATCH_DOG_REQ,
	GW_SYS_STOP_WATCH_DOG_REQ,
	GW_SYS_SET_TELNET_REQ,
	GW_SYS_GET_TELNET_REQ,
	GW_SYS_GET_RTSP_INFO_REQ
};

/*****************************************************************************/
/*  task GW_CONSOLE define
 */
/*****************************************************************************/
/* define timer */

/* define signal */
enum {
	GW_CONSOLE_INTERNAL_LOGIN_CMD = AK_USER_DEFINE_SIG,
};

/*****************************************************************************/
/*  task GW_TASK_WEBRTC define
 */
/*****************************************************************************/
/* define timer */
#define GW_WEBRTC_RECONNECT_WEBSOCKET_INTERVAL			   (5000)
/* define signal */
enum {
	GW_WEBRTC_SIGNALING_WEBSOCKET_REQ = AK_USER_DEFINE_SIG,

	GW_WEBRTC_RECONNECT_WEBSOCKET_REG,
	GW_WEBRTC_SET_SIGNALING_WEBSOCKET_REG,
	GW_WEBRTC_GET_SIGNALING_WEBSOCKET_REG,

	GW_WEBRTC_CREATE_PEER_REG
};
/*****************************************************************************/
/*  task GW_TASK_MQTT define
 */
/*****************************************************************************/
/* define timer */
#define GW_RECONNECT_MQTT_INTERVAL			   (5000)
/* define signal */
enum {
    GW_WETRTC_CONNECT_MQTT_REG = AK_USER_DEFINE_SIG,
	GW_WETRTC_DISCONNECT_MQTT_REG,
	GW_WEBRTC_RECONNECT_MQTT_REG,
	GW_WEBRTC_SET_SIGNALING_MQTT_REG,
	GW_WEBRTC_GET_SIGNALING_MQTT_REG,
};

/*****************************************************************************/
/*  task GW_TASK_AV define
 */
/*****************************************************************************/
/* define timer */

/* define signal */
enum {
	GW_AV_INIT_REQ = AK_USER_DEFINE_SIG,
	GW_AV_DEINIT_REQ,
	GW_AV_START_REQ,
	GW_AV_STOP_REQ
};
/*****************************************************************************/
/*  global define variable
 */
/*****************************************************************************/
#define APP_OK (0x00)
#define APP_NG (0x01)

#define APP_FLAG_OFF (0x00)
#define APP_FLAG_ON	 (0x01)

#endif	  // __APP_H__
