#include "timer.h"
#include "app_config.h"
#include "task_list.h"

#include "task_webrtc.h"
#include "task_av.h"
#include "task_mqtt.h"

ak_task_t task_list[] = {
	/* SYSTEM TASKS */
	{	AK_TASK_TIMER_ID,	   	TASK_PRI_LEVEL_1, 	timer_entry,			  	&timer_mailbox,			  		"timer service"	 	},

 /* APP TASKS */
 	{	GW_TASK_WEBRTC_ID,		TASK_PRI_LEVEL_6, 	gw_task_webrtc_entry,		&gw_task_webrtc_mailbox,	 	"webrtc task"		 	},
	{	GW_TASK_MQTT_ID,		TASK_PRI_LEVEL_6,	gw_task_mqtt_entry,			&gw_task_mqtt_mailbox,			",mqtt task"			},
#if AV_ENABLE == 1
	{	GW_TASK_AV_ID,			TASK_PRI_LEVEL_6, 	gw_task_av_entry,			&gw_task_av_mailbox,	 		"webrtc task"		 	},
#endif
};
