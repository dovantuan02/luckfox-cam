#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__

#include "ak.h"
#include "message.h"
#include "app_config.h"

/** default if_des_type when get pool memory
 * this define MUST BE coresponding with app.
 */
#define AK_APP_TYPE_IF 101

enum {
	/* SYSTEM TASKS */
	AK_TASK_TIMER_ID,

	/* APP TASKS */
	GW_TASK_WEBRTC_ID,
	GW_TASK_MQTT_ID,
#if AV_ENABLE == 1
	GW_TASK_AV_ID,
#endif
	/* EOT task ID */
	AK_TASK_LIST_LEN,
};

extern ak_task_t task_list[];

#endif	  //__TASK_LIST_H__
