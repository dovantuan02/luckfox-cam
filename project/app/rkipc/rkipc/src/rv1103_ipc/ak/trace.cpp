/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   17/01/2018
 * @brief: ak trace
 ******************************************************************************
 **/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/un.h>
#include <errno.h>
#include <queue>

#include "message.h"
#include "trace.h"
#include "ak.h"

using namespace std;

typedef enum {
	AK_TRACE_MSG_HEADER_TYPE_NONE,
	AK_TRACE_MSG_HEADER_TYPE_PUT,
	AK_TRACE_MSG_HEADER_TYPE_GET,
	AK_TRACE_MSG_HEADER_TYPE_FREE,
} ak_trace_msg_header_type_e;

typedef enum {
	AK_TRACE_MSG_HEADER_SUB_TYPE_NONE,
} ak_trace_msg_header_sub_type_e;

typedef struct {
	/* task header */
	uint32_t src_task_id;
	uint32_t des_task_id;
	uint32_t sig;

	/* external task header */
	uint32_t if_src_task_id;
	uint32_t if_des_task_id;
	uint32_t if_src_type;
	uint32_t if_des_type;
	uint32_t if_sig;

	/* message type (pool type)*/
	uint32_t type;

	/* payload */
	uint32_t len;
} ak_msg_header_t;

typedef struct {
	uint32_t type;
	uint32_t sub_type;
	uint32_t allocate;
	time_t time;
} ak_trace_msg_header_t;

typedef struct {
	ak_trace_msg_header_t ak_trace_msg_header;
	ak_msg_header_t ak_msg_header;
	uint8_t *ak_msg_payload;
} ak_trace_msg_t;

static int trace_target_socket_fd;
static const char *trace_target_socket_path = "/tmp/ak_trace_target";

int trace_host_socket_fd;
static const char *trace_host_socket_path = "/tmp/ak_trace_host";

static int trace_post_msg(ak_trace_msg_t *, ak_msg_t *);

static bool trace_task_started = false;
static pthread_mutex_t mt_trace_task_started;

static pthread_t trace_task;
static void *trace_task_handler(void *);

static pthread_mutex_t mt_trace_queue_msg_access;

typedef struct {
	uint8_t *data;
	uint32_t len;
} msg_raw_data_t;

static queue<msg_raw_data_t> trace_queue_msg;

uint8_t trace_msg_init() {
	struct sockaddr_un addr;

	/* create gateway server socket */
	trace_target_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (trace_target_socket_fd == -1) {
		return RET_TRACE_ERR_INIT_SOCKET;
	}

	/* remove exist path */
	if (remove(trace_target_socket_path) == -1 && errno != ENOENT) {}

	/* bind socket */
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, trace_target_socket_path, sizeof(addr.sun_path) - 1);
	if (bind(trace_target_socket_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
		return RET_TRACE_ERR_INIT_BIND;
	}

	/* start listen socket */
	if (listen(trace_target_socket_fd, 5) == -1) {
		return RET_TRACE_ERR_INIT_LISTEN;
	}

	trace_host_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (trace_host_socket_fd < 0) {
		return RET_TRACE_ERR_INIT_HOST_SOCKET;
	}

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;

	strncpy(addr.sun_path, trace_host_socket_path, sizeof(addr.sun_path) - 1);
	if (connect(trace_host_socket_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
		return RET_TRACE_ERR_INIT_HOST_CONNECT;
	}

	pthread_create(&trace_task, NULL, trace_task_handler, NULL);

	bool thread_started = false;

	do {
		pthread_mutex_lock(&mt_trace_task_started);
		thread_started = trace_task_started;
		pthread_mutex_unlock(&mt_trace_task_started);
	} while (!thread_started);

	return RET_TRACE_OK;
}

void *trace_task_handler(void *) {
	pthread_mutex_lock(&mt_trace_task_started);
	trace_task_started = true;
	pthread_mutex_unlock(&mt_trace_task_started);

	msg_raw_data_t raw_data;

	while (1) {
		raw_data.data = NULL;
		raw_data.len  = 0;

		pthread_mutex_lock(&mt_trace_queue_msg_access);
		if (!trace_queue_msg.empty()) {
			raw_data = trace_queue_msg.front();
			trace_queue_msg.pop();
		}
		pthread_mutex_unlock(&mt_trace_queue_msg_access);

		if (raw_data.data != NULL) {
			if (write(trace_host_socket_fd, raw_data.data, raw_data.len) != (ssize_t)raw_data.len) {
				free(raw_data.data);
				close(trace_host_socket_fd);
				return (NULL);
			}
			free(raw_data.data);
		}
		usleep(1);
	}

	return (NULL);
}

void trace_msg_put(ak_msg_t *msg, uint32_t allocate, time_t time) {
	if (trace_task_started) {
		ak_trace_msg_t *ak_trace_msg			   = (ak_trace_msg_t *)malloc(sizeof(ak_trace_msg_t));
		ak_trace_msg->ak_trace_msg_header.type	   = AK_TRACE_MSG_HEADER_TYPE_PUT;
		ak_trace_msg->ak_trace_msg_header.sub_type = AK_TRACE_MSG_HEADER_SUB_TYPE_NONE;
		ak_trace_msg->ak_trace_msg_header.allocate = allocate;
		ak_trace_msg->ak_trace_msg_header.time	   = time;
		trace_post_msg(ak_trace_msg, msg);
		free(ak_trace_msg);
	}
}

void trace_msg_get(ak_msg_t *msg, uint32_t allocate, time_t time) {
	if (trace_task_started) {
		ak_trace_msg_t *ak_trace_msg			   = (ak_trace_msg_t *)malloc(sizeof(ak_trace_msg_t));
		ak_trace_msg->ak_trace_msg_header.type	   = AK_TRACE_MSG_HEADER_TYPE_GET;
		ak_trace_msg->ak_trace_msg_header.sub_type = AK_TRACE_MSG_HEADER_SUB_TYPE_NONE;
		ak_trace_msg->ak_trace_msg_header.allocate = allocate;
		ak_trace_msg->ak_trace_msg_header.time	   = time;
		trace_post_msg(ak_trace_msg, msg);
		free(ak_trace_msg);
	}
}

void trace_msg_free(ak_msg_t *msg, uint32_t allocate, time_t time) {
	if (trace_task_started) {
		ak_trace_msg_t *ak_trace_msg			   = (ak_trace_msg_t *)malloc(sizeof(ak_trace_msg_t));
		ak_trace_msg->ak_trace_msg_header.type	   = AK_TRACE_MSG_HEADER_TYPE_FREE;
		ak_trace_msg->ak_trace_msg_header.sub_type = AK_TRACE_MSG_HEADER_SUB_TYPE_NONE;
		ak_trace_msg->ak_trace_msg_header.allocate = allocate;
		ak_trace_msg->ak_trace_msg_header.time	   = time;
		trace_post_msg(ak_trace_msg, msg);
		free(ak_trace_msg);
	}
}

int trace_post_msg(ak_trace_msg_t *trace_msg, ak_msg_t *msg) {
	uint32_t trace_msg_len = sizeof(ak_trace_msg_header_t) + sizeof(ak_msg_header_t) + msg->header->len;

	uint8_t *msg_sending_packet = (uint8_t *)malloc(trace_msg_len);

	memcpy(msg_sending_packet, &trace_msg->ak_trace_msg_header, sizeof(ak_trace_msg_header_t));

	ak_msg_header_t st_ak_msg_header;
	st_ak_msg_header.src_task_id	= msg->header->src_task_id;
	st_ak_msg_header.des_task_id	= msg->header->des_task_id;
	st_ak_msg_header.sig			= msg->header->sig;
	st_ak_msg_header.if_src_task_id = msg->header->if_src_task_id;
	st_ak_msg_header.if_des_task_id = msg->header->if_des_task_id;
	st_ak_msg_header.if_src_type	= msg->header->if_src_type;
	st_ak_msg_header.if_des_type	= msg->header->if_des_type;
	st_ak_msg_header.if_sig			= msg->header->sig;
	st_ak_msg_header.type			= msg->header->type;
	st_ak_msg_header.len			= msg->header->len;
	memcpy((uint8_t *)(msg_sending_packet + sizeof(ak_trace_msg_header_t)), (uint8_t *)&st_ak_msg_header, sizeof(ak_msg_header_t));

	if (msg->header->len) {
		memcpy((uint8_t *)(msg_sending_packet + sizeof(ak_trace_msg_header_t) + sizeof(ak_msg_header_t)), (uint8_t *)msg->header->payload, msg->header->len);
	}

	msg_raw_data_t raw_data;
	raw_data.data = msg_sending_packet;
	raw_data.len  = trace_msg_len;

	pthread_mutex_lock(&mt_trace_queue_msg_access);
	trace_queue_msg.push(raw_data);
	pthread_mutex_unlock(&mt_trace_queue_msg_access);

	return 1;
}
