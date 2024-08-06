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
#define GW_WEBRTC_ERASE_CLIENT_NO_ANSWER_TIMEOUT_INTERVAL (40000) /* 40s */
#define GW_WEBRTC_TRY_CONNECT_SOCKET_INTERVAL			  (10000) /* 10s */
#define GW_WEBRTC_TRY_GET_EXTERNAL_IP_INTERVAL			  (7000)  /* 7s */
#define GW_WEBRTC_WAIT_REQUEST_TIMEOUT_INTERVAL			  (20000) /* 20s */
#define GW_WEBRTC_CLIENT_SEND_PING_INTERVAL				  (10000) /* 10s */
#define GW_WEBRTC_ERASE_CLIENT_PING_PONG_TIMEOUT_INTERVAL (20000) /* 20s */
#define GW_WEBRTC_RELEASE_CLIENT_PUSH_TO_TALK_INTERVAL	  (2500)  /* 2.5s */

#define GW_WEBRTC_RECONNECT_WEBSOCKET_INTERVAL			   (5000)
/* define signal */
enum {
	GW_WEBRTC_SIGNALING_MQTT_REQ = AK_USER_DEFINE_SIG,

	GW_WEBRTC_RECONNECT_WEBSOCKET_REG,
	GW_WEBRTC_SET_SIGNALING_WEBSOCKET_REG,
	GW_WEBRTC_GET_SIGNALING_WEBSOCKET_REG,

	GW_WEBRTC_SET_SIGNLING_SERVER_REQ,
	GW_WEBRTC_GET_SIGNLING_SERVER_REQ,
	GW_WEBRTC_CHECK_CLIENT_CONNECTED_REQ,
	GW_WEBRTC_ERASE_CLIENT_REQ,
	GW_WEBRTC_DBG_IPC_SEND_MESSAGE_REQ,
	GW_WEBRTC_ON_MESSAGE_CONTROL_DATACHANNEL_REQ,
	GW_WEBRTC_DATACHANNEL_DOWNLOAD_RELEASE_REQ,
	GW_WEBRTC_RELEASE_CLIENT_PUSH_TO_TALK,
	GW_WEBRTC_TRY_GET_STUN_EXTERNAL_IP_REQ,
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
