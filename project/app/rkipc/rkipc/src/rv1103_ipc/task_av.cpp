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
#include "task_av.h"

#include "stream.hpp"

// ROCKIT
#include "audio.h"
#include "video.h"
#include "common.h"
#include "isp.h"
#include "log.h"
#include "network.h"
#include "param.h"
#include "rockiva.h"
#include "server.h"
#include "storage.h"
#include "system.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rkipc.c"

enum { LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_NOTICE };

int enable_minilog = 0;
int rkipc_log_level = LOG_INFO;

char *rkipc_ini_path_ = "/userdata/rkipc.ini";
char *rkipc_iq_file_path_ = "/etc/iqfiles";

q_msg_t gw_task_av_mailbox;

void *gw_task_av_entry(void *) {
#if AV_ENABLE == 1
	ak_msg_t *msg = AK_MSG_NULL;

	wait_all_tasks_started();
	APP_DBG("[STARTED] gw_task_av_entry\n");

	while (1) {
		/* get messge */
		msg = ak_msg_rev(GW_TASK_AV_ID);

		switch (msg->header->sig) {
			case GW_AV_INIT_REQ: {
				APP_DBG_SIG("GW_AV_INIT_REQ\r\n");	
				APP_DBG("rkipc_ini_path_ is %s, rkipc_iq_file_path_ is %s, rkipc_log_level is %d\n",
						rkipc_ini_path_, rkipc_iq_file_path_, rkipc_log_level);

				rk_param_init(rkipc_ini_path_);
				rk_network_init(NULL);
				rk_system_init();

				rk_param_set_int("video.source:enable_ivs", 0);
				rk_param_set_int("video.source:enable_npu", 0);
				rk_param_set_int("video.source:enable_aiq", 1);

				if (rk_param_get_int("video.source:enable_npu", 0)) {
					rkipc_rockiva_init();
				}
				if (rk_param_get_int("video.source:enable_aiq", 1)) {
					rk_isp_init(0, rkipc_iq_file_path_);
					rk_isp_set_frame_rate(0, rk_param_get_int("isp.0.adjustment:fps", 30));
					if (rk_param_get_int("isp:init_form_ini", 1)) {
						rk_isp_set_from_ini(0);
					}
				}
				RK_MPI_SYS_Init();

				rk_param_set_int("video.source:enable_rtsp", 0);		// disable rtsp
				rk_param_set_int("video.source:enable_venc_0", 1);		
				rk_param_set_int("video.source:enable_venc_1", 0);		// disable vnc 1

				// config video 0
				rk_video_set_output_data_type(1, "H.264");		// config to h.264 venc 1
				rk_param_set_int("video.1:width", 704);
			    rk_param_set_int("video.1:height", 576);

				// config video 0
				rk_video_set_output_data_type(0, "H.264");		// config to h.264 venc 0
				rk_param_set_int("video.0:width", 1280);
				rk_param_set_int("video.0:height", 800);

				rk_param_set_int("audio.0:height", 800);

				rk_video_init();
				if (rk_param_get_int("audio.0:enable", 0)) {
					APP_DBG("Audio 0 Enable\r\n");
					rkipc_audio_init();
				}
				rkipc_server_init();
				rk_storage_init();
			} break;

			case GW_AV_DEINIT_REQ: {
				APP_DBG_SIG("GW_AV_DEINIT_REQ\r\n");
				rk_storage_deinit();
				rkipc_server_deinit();
				rk_system_deinit();
				rk_video_deinit();
				if (rk_param_get_int("video.source:enable_aiq", 1))
					rk_isp_deinit(0);
				if (rk_param_get_int("audio.0:enable", 0))
					rkipc_audio_deinit();
				RK_MPI_SYS_Exit();
				if (rk_param_get_int("video.source:enable_npu", 0))
					rkipc_rockiva_deinit();
				rk_video_deinit();
				rk_network_deinit();
				rk_param_deinit();
			} break;

			case GW_AV_START_REQ: {
				APP_DBG_SIG("GW_AV_START_REQ\r\n");
				// Stream::start_stream();
			} break;

			case GW_AV_STOP_REQ: {
				APP_DBG_SIG("GW_AV_STOP_REQ\r\n");
				Stream _stream;
				Stream::stop_stream(_stream);
			} break;
		}
	}
#endif // AV_ENABLE
}


