#ifndef _TASK_MQTT_H_
#define _TASK_MQTT_H_

#include "message.h"

extern q_msg_t gw_task_mqtt_mailbox;
extern void *gw_task_mqtt_entry(void *);

#endif // _TASK_MQTT_H_
