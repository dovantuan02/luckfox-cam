#include "app.h" 
#include "app_data.h"
#include "app_config.h"
#include "app_dbg.h"
#include "task_list.h"

#include "mqtt.hpp"

using namespace std;

static int MQTT_message_arrived(void* context, char* topcic, int topic_len, MQTTClient_message* m) {
	APP_DBG("-----%s------\r\n", __func__);

	std::string payload((char*)m->payload, m->payloadlen);
	// APP_DBG("Payload : %s\r\n", payload.c_str());
	task_post_dynamic_msg(GW_TASK_MQTT_ID, 
							GW_WEBRTC_GET_SIGNALING_MQTT_REG, 
							(uint8_t*)payload.data(),payload.size());

	MQTTClient_free(topcic);
	MQTTClient_freeMessage(&m);
	return 1;
}

static void MQTT_delivery_complete(void* context, MQTTClient_deliveryToken dt) {
	APP_DBG("-----%s------\r\n", __func__);
}

MQTT::MQTT(string host_url, uint16_t port, string client_id) {
	MQTTClient_create(&client, host_url.data(), client_id.data(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
	MQTTClient_setCallbacks(client, NULL, NULL, MQTT_message_arrived, MQTT_delivery_complete);
}

MQTT::~MQTT() { }

bool MQTT::MQTT_connnect(string user, string pass) {
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	conn_opts.keepAliveInterval = 10;
	conn_opts.cleansession = 1;
	conn_opts.username = user.data();
	conn_opts.password = pass.data();
	return (MQTTClient_connect(client, &conn_opts) == MQTTCLIENT_SUCCESS);
}

void MQTT::MQTT_disconnect() {
	MQTTClient_disconnect(client, 0);
	MQTTClient_destroy(&client);
}

bool MQTT::MQTT_subcribe(string topic, uint8_t qos) {
	return (MQTTClient_subscribe(client, topic.data(), qos) == MQTTCLIENT_SUCCESS);
}

bool MQTT::MQTT_unsubcribe(string topic) {
	return (MQTTClient_unsubscribe(client, topic.data()) == MQTTCLIENT_SUCCESS);
}

bool MQTT::MQTT_pulish(string topic , string data, uint8_t qos) {
	MQTTClient_message pub_message = MQTTClient_message_initializer;
	pub_message.payload = (void*)data.data();
	pub_message.payloadlen = data.size();
	pub_message.qos = qos;
	pub_message.retained = 0;
	return (MQTTClient_publish(client, topic.data(), pub_message.payloadlen, pub_message.payload,
								pub_message.qos, pub_message.retained, NULL) == MQTTCLIENT_SUCCESS);
}

bool MQTT::MQTT_connection_state() {
	return (MQTTClient_isConnected(client));
}