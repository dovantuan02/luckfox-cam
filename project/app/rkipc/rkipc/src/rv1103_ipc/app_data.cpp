#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstdlib>

#include "app_data.h"
#include "app_dbg.h"

#include "stream.hpp"

#include <pthread.h>

#include <rk_mpi_venc.h>
#include <rk_mpi_vi.h>
#include <rk_mpi_sys.h>
#include <rk_mpi_mb.h>

using namespace std;

static void stream_get_data(uint8_t *data, uint32_t len) {
	// APP_DBG("----------%s-----------\r\n", __func__);
	Stream::stream_video(data, len);
}

void venc_0_handler(void *data, uint32_t len) {
	// APP_DBG("----%s-----\r\n", __func__);
	stream_get_data((uint8_t*)data, len);
}

void venc_1_handler(void *data, uint32_t len) {
	APP_DBG("----%s-----\r\n", __func__);
}

