#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>

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

#define MQTT_PUB_TOPIC		"luckfox_pub"
#define MQTT_SUB_TOPIC		"luckfox_sub"
#define SIGNALING_QOS		(0)
#define SIGNALING_CLIENT_ID	"signaling_luckfox"

#define MQTT_USER			""
#define MQTT_PASS			""

#define MQTT_METHOD			"SET"
#define MQTT_WS_SET			"WebSocketServer"

void mqtt_message_set(json data);
string command(string command);
q_msg_t gw_task_mqtt_mailbox;

void *gw_task_mqtt_entry(void *) {
	ak_msg_t *msg = AK_MSG_NULL;

	wait_all_tasks_started();
	
	APP_DBG("[STARTED] %s\n", __func__);

	auto p_mqtt = make_shared<MQTT>(MQTT_HOST, MQTT_PORT, SIGNALING_CLIENT_ID);
	task_post_pure_msg(GW_TASK_MQTT_ID, GW_MQTT_CONNECT_REG);
	while(1) {
		/* get messge */
		msg = ak_msg_rev(GW_TASK_MQTT_ID);

		switch (msg->header->sig)
		{
			case GW_MQTT_CONNECT_REG: {
				APP_DBG("GW_MQTT_CONNECT_REG\r\n");
				
				if (!p_mqtt->MQTT_connnect(MQTT_USER, MQTT_PASS)) {
					APP_DBG("MQTT connection error !\r\n");
					timer_set(GW_TASK_MQTT_ID, GW_MQTT_CONNECT_REG, 1000, TIMER_ONE_SHOT);
					break;
				}
				if (!p_mqtt->MQTT_subcribe(MQTT_SUB_TOPIC, SIGNALING_QOS)) {
					APP_DBG("MQTT subcribe error !\r\n");
					break;
				}
				APP_DBG("MQTT connect success \r\n");
			} break;
			
			case GW_MQTT_DISCONNECT_REG: {
				APP_DBG("GW_MQTT_DISCONNECT_REG\r\n");
				p_mqtt->MQTT_unsubcribe(MQTT_SUB_TOPIC);
				p_mqtt->MQTT_disconnect();
			} break;

			case GW_MQTT_PUB_MESS_REG: {
				APP_DBG("GW_MQTT_PUB_MESS_REG\r\n");
				std::string data((char*)msg->header->payload, msg->header->len);

				p_mqtt->MQTT_pulish(MQTT_PUB_TOPIC, data, SIGNALING_QOS);
			} break;

			case GW_MQTT_ON_MESS_REG: {
				APP_DBG("%s\r\n", command("ls").c_str());
				// cout << command("ls") << endl;
				std::string message((char*)msg->header->payload, msg->header->len);
				json data = json::parse(message);
				auto it = data.find("Method");
				if(it == data.end()) {
					break;
				}
				string method = it->get<string>();
				if (method == MQTT_METHOD) {
					mqtt_message_set(data);
				}
			} break;

			case GW_MQTT_CHECK_CONNECT_REG: {
				APP_DBG("GW_MQTT_CHECK_CONNECT_REG\r\n");
				if (!p_mqtt->MQTT_connection_state()) {
					p_mqtt->MQTT_disconnect();
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

void mqtt_message_set(json json_data) {
	auto it = json_data.find("MessageType");
	if (it == json_data.end()) {
		return;
	}
	if (it->get<string>() == MQTT_WS_SET) {
		try
		{
			if (json_data.contains("Data") && json_data["Data"].contains("SocketUrl")) {
				std::string socketUrl = json_data["Data"]["SocketUrl"].get<std::string>();
				APP_DBG("SocketUrl is valid :%s\r\n", socketUrl.data());
				task_post_dynamic_msg(GW_TASK_WEBRTC_ID, 
										GW_WEBRTC_WEBSOCKET_REQ,
										(uint8_t*)socketUrl.data(),
										socketUrl.size());
			}
			else {
				APP_LOG_WARNING("SocketUrl invalid \r\n");
			}
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
	}
}

string command(string command) {
   char buffer[128];
   string result = "";

   // Open pipe to file
   FILE* pipe = popen(command.c_str(), "r");
   if (!pipe) {
      return "popen failed!";
   }

   // read till end of process:
   while (!feof(pipe)) {
      // use buffer to read and add to result
      if (fgets(buffer, 128, pipe) != NULL)
         result += buffer;
   }

   pclose(pipe);
   return result;
}