#ifndef __TASK_WEBRTC_H__
#define __TASK_WEBRTC_H__

#include <iostream>
#include <optional>

#include "message.h"

#include "opusfileparser.hpp"
#include "helpers.hpp"
#include "stream.hpp"

#include "app_config.h"

using namespace std;

extern optional<shared_ptr<Stream>> avStream;
extern unordered_map<string, shared_ptr<Client>> clients;
extern q_msg_t gw_task_webrtc_mailbox;
extern void *gw_task_webrtc_entry(void *);

#endif	  //__TASK_WEBRTC_H__
