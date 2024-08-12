#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iomanip>
#include <cstdio>
#include <vector>
#include <variant>

#include <algorithm>
#include <future>
#include <iostream>
#include <memory>
// #include <random>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <chrono>
#include <pthread.h>
#include <assert.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <atomic>

#include "app.h" 
#include "app_data.h"
#include "app_config.h"
#include "app_dbg.h"
#include "task_list.h"
#include "json.hpp"
#include "task_mqtt.h"
#include "task_webrtc.h"

#include "rtc/rtc.hpp"

#include "mqtt.hpp"

using namespace std;
using namespace rtc;
using json = nlohmann::json;

template <class T> weak_ptr<T> make_weak_ptr(shared_ptr<T> ptr) { return ptr; }

#define MQTT_HOST			"mqtt://test.mosquitto.org"
#define MQTT_PORT			(1883)

#define SIGNALING_TOPIC		"signaling_webrtc"
#define SIGNALING_QOS		(0)
#define SIGNALING_CLIENT_ID	"signaling_luckfox"

#define MQTT_USER			""
#define MQTT_PASS			""

q_msg_t gw_task_mqtt_mailbox;

void *gw_task_mqtt_entry(void *) {
	ak_msg_t *msg = AK_MSG_NULL;

	wait_all_tasks_started();
	
	APP_DBG("[STARTED] gw_task_mqtt_entry\n");

	auto p_mqtt = make_shared<MQTT>(MQTT_HOST, MQTT_PORT, SIGNALING_CLIENT_ID);
	timer_set(GW_TASK_MQTT_ID, GW_WETRTC_CONNECT_MQTT_REG, 100, TIMER_ONE_SHOT);
	while(1) {
		/* get messge */
		msg = ak_msg_rev(GW_TASK_MQTT_ID);

		switch (msg->header->sig)
		{
			case GW_WETRTC_CONNECT_MQTT_REG: {
				APP_DBG("GW_WETRTC_CONNECT_MQTT_REG\r\n");
				if (!p_mqtt->MQTT_connnect(MQTT_USER, MQTT_PASS)) {
					APP_DBG("MQTT connection error !\r\n");
					break;
				}
				if (!p_mqtt->MQTT_subcribe(SIGNALING_TOPIC, SIGNALING_QOS)) {
					APP_DBG("MQTT subcribe error !\r\n");
					break;
				}
				APP_DBG("MQTT connect success \r\n");
			} break;
			
			case GW_WETRTC_DISCONNECT_MQTT_REG: {
				APP_DBG("GW_WETRTC_DISCONNECT_MQTT_REG\r\n");
				p_mqtt->MQTT_unsubcribe(SIGNALING_TOPIC);
				p_mqtt->MQTT_disconnect();
			} break;
			
			case GW_WEBRTC_RECONNECT_MQTT_REG: {
				APP_DBG("GW_WEBRTC_RECONNECT_MQTT_REG\r\n");
			} break;

			case GW_WEBRTC_SET_SIGNALING_MQTT_REG: {
				APP_DBG("GW_WEBRTC_SET_SIGNALING_MQTT_REG\r\n");
				std::string data((char*)msg->header->payload, msg->header->len);

				p_mqtt->MQTT_pulish(SIGNALING_TOPIC, data, SIGNALING_QOS);
			} break;

			case GW_WEBRTC_GET_SIGNALING_MQTT_REG: {
				APP_DBG("GW_WEBRTC_GET_SIGNALING_MQTT_REG\r\n");
				std::string data((char*)msg->header->payload, msg->header->len);
				json message = json::parse(data);
				
				auto it =  message.find("id");
				if(it == message.end()) {
					break;
				}
				string id = it->get<string>();
				APP_DBG("id : %s\n", id.c_str());

				it = message.find("type");
				if(it == message.end()) {
					break;
				}
				string type = it->get<string>();
				APP_DBG("type : %s\n", type.c_str());
				if(type == "request") {
					APP_DBG("Receive Request !\r\n");
					Configuration config;
					config.iceServers.emplace_back((char*)"stun:stun.l.google.com:19302");  // config sturn server
					config.disableAutoNegotiation = true;
					clients.emplace(id, createPeerConnection(config, make_weak_ptr(p_mqtt) , id));
				} 
				else if (type == "answer") {
					if (auto jt = clients.find(id); jt != clients.end()) {
						auto pc = jt->second->peerConnection;
						auto sdp = message["sdp"].get<string>();
						auto description = Description(sdp, type);
						pc->setRemoteDescription(description);
						APP_DBG("Video Start \r\n");
#if AV_ENABLE == 1
						task_post_pure_msg(GW_TASK_AV_ID, GW_AV_START_REQ);
#endif // AV_ENABLE
					}
				}
			} break;

			default:
				break;
		}
		/* free message */
		ak_msg_free(msg);
	}
	return (void *)0;
}