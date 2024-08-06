#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include "ak.h"

#include "app.h"
#include "app_dbg.h"
#include "app_config.h"
#include "app_data.h"

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

void exit_app(int ret);

void task_init() {
	signal(SIGINT, exit_app);
	signal(SIGQUIT, exit_app);
	signal(SIGTERM, exit_app);
	signal(SIGKILL, exit_app);
}

void exit_app(int ret) {
	APP_DBG("------%s-------\r\n", __func__);

#if AV_ENABLE == 1
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
	rk_network_deinit();
	rk_param_deinit();
#endif

	exit(ret);
}
