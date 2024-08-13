#ifndef _MQTT_H_
#define _MQTT_H_

#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "MQTTClient.h"

using namespace std;

class MQTT {
public:
	MQTT(string host_url, uint16_t port, string client_id);
	~MQTT();
	bool MQTT_connnect(string user, string pass);
	void MQTT_disconnect();
	bool MQTT_subcribe(string topic, uint8_t qos);
	bool MQTT_unsubcribe(string topic);
	bool MQTT_pulish(string topic , string data, uint8_t qos);
	bool MQTT_connection_state();
	void MQTT_conn_lost_handle(void* , char* );
protected:
	MQTTClient client;
private:
	string mqtt_user;
	string mqtt_pass;
};

#endif 