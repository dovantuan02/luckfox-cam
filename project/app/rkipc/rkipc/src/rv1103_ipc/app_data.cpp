#include "app_data.h"
#include "app_dbg.h"

#include "stream.hpp"

#include <pthread.h>

#include <rk_mpi_venc.h>
#include <rk_mpi_vi.h>
#include <rk_mpi_sys.h>
#include <rk_mpi_mb.h>

pthread_mutex_t g_mutex_strem;

static void stream_get_data(uint8_t *data, uint32_t len) {
	// APP_DBG("----------%s-----------\r\n", __func__);
	pthread_mutex_lock(&g_mutex_strem);
	Stream::stream_video(data, len);
	pthread_mutex_unlock(&g_mutex_strem);
}

void venc_0_handler(void *data, uint32_t len) {
	// APP_DBG("----%s-----\r\n", __func__);
	stream_get_data((uint8_t*)data, len);
}

void venc_1_handler(void *data, uint32_t len) {
	APP_DBG("----%s-----\r\n", __func__);
}