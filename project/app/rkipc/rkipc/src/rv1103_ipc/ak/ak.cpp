/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   12/01/2017
 * @brief:  Main defination of active kernel
 ******************************************************************************
 **/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
// #include <curl/curl.h>

#include "ak.h"
#include "ak_dbg.h"
#include "message.h"

#include "sys_dbg.h"

#include "task_list.h"

static uint32_t ak_thread_table_len = AK_TASK_LIST_LEN;

static pthread_mutex_t mt_ak_thread_started;
static uint32_t ak_thread_started = 0;

int main() {
	int status;

	AK_MSG_DBG("TASK LIST LEN: %d\n", ak_thread_table_len);

	task_init();

	/* Must initialize libcurl before any threads are started */
	// curl_global_init(CURL_GLOBAL_DEFAULT); //TODO

#if (AK_PRIORITY_ENABLE == 1)
	struct sched_param thread_sched_param;
#endif

	for (uint32_t index = 0; index < ak_thread_table_len; index++) {
		/* init mailbox */
		q_msg_init(task_list[index].mailbox);

		pthread_attr_init(&(task_list[index].pthread_attr));

#if (AK_PRIORITY_ENABLE == 1)
		// APP_SOCKET_DBG(DBG_TAG, "Priority min: %d, max:%d\n", sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
		status = pthread_attr_setschedpolicy(&(task_list[index].pthread_attr), SCHED_FIFO);
		if (status != 0) {
			AK_MSG_DBG("pthread_attr_setschedpolicy ERR=%d\n", status);
		}

		thread_sched_param.sched_priority = task_list[index].pri;
		status							  = pthread_attr_setschedparam(&(task_list[index].pthread_attr), &thread_sched_param);
		if (status != 0) {
			AK_MSG_DBG("pthread_attr_setschedparam ERR=%d\n", status);
		}
#endif

		/* create task */
		pthread_create(&(task_list[index].pthread), &(task_list[index].pthread_attr), task_list[index].task, NULL);
		pthread_setname_np(task_list[index].pthread, task_list[index].info);
#if (AK_PRIORITY_ENABLE == 1)
		AK_PRINT("ID:%08x  PRI:%02d  CREATE: %s\n", (uint32_t)task_list[index].pthread, thread_sched_param.sched_priority, task_list[index].info);
#else
		AK_PRINT("ID:%08x\tCREATE: %s\n", (uint32_t)task_list[index].pthread, task_list[index].info);
#endif
		/* create queue trigger */
		pthread_cond_init(&task_list[index].mailbox_cond, NULL);
	}

	for (uint32_t index = 0; index < ak_thread_table_len; index++) {
		pthread_join(task_list[index].pthread, NULL);
	}

	// curl_global_cleanup();
	return 0;
}

void wait_all_tasks_started() {
	bool check_ret = true;

	pthread_mutex_lock(&mt_ak_thread_started);
	ak_thread_started++;
	pthread_mutex_unlock(&mt_ak_thread_started);

	while (check_ret) {
		pthread_mutex_lock(&mt_ak_thread_started);

		if (ak_thread_started < ak_thread_table_len) {
			check_ret = true;
		}
		else {
			check_ret = false;
		}

		pthread_mutex_unlock(&mt_ak_thread_started);
		usleep(100);
	}
	AK_PRINT("AK started !!\n");
}

ak_msg_t *get_pure_msg() {
	ak_msg_t *g_msg = (ak_msg_t *)malloc(sizeof(ak_msg_t));
	if (g_msg == NULL) {
		FATAL("AK", 0x01);
	}

	g_msg->header = (header_t *)malloc(sizeof(header_t));
	if (g_msg->header == NULL) {
		FATAL("AK", 0x02);
	}

	g_msg->header->if_des_type	  = AK_APP_TYPE_IF;
	g_msg->header->if_sig		  = 0xFFFFFFFF;
	g_msg->header->if_src_task_id = 0xFFFFFFFF;
	g_msg->header->if_des_task_id = 0xFFFFFFFF;

	g_msg->header->type	   = PURE_MSG_TYPE;
	g_msg->header->len	   = 0;
	g_msg->header->payload = NULL;

	AK_MSG_DBG("[MSG] get msg:%p\theader:%p\n", g_msg, g_msg->header);
	return g_msg;
}

ak_msg_t *get_dynamic_msg() {
	ak_msg_t *g_msg = (ak_msg_t *)malloc(sizeof(ak_msg_t));
	if (g_msg == NULL) {
		FATAL("AK", 0x02);
	}

	g_msg->header = (header_t *)malloc(sizeof(header_t));
	if (g_msg->header == NULL) {
		FATAL("AK", 0x03);
	}

	g_msg->header->if_des_type	  = AK_APP_TYPE_IF;
	g_msg->header->if_sig		  = 0xFFFFFFFF;
	g_msg->header->if_src_task_id = 0xFFFFFFFF;
	g_msg->header->if_des_task_id = 0xFFFFFFFF;

	g_msg->header->type	   = DYNAMIC_MSG_TYPE;
	g_msg->header->len	   = 0;
	g_msg->header->payload = NULL;

	AK_MSG_DBG("[MSG] get msg:%p\theader:%p\n", g_msg, g_msg->header);
	return g_msg;
}

ak_msg_t *get_common_msg() {
	ak_msg_t *g_msg = (ak_msg_t *)malloc(sizeof(ak_msg_t));
	if (g_msg == NULL) {
		FATAL("AK", 0x04);
	}

	g_msg->header = (header_t *)malloc(sizeof(header_t));
	if (g_msg->header == NULL) {
		FATAL("AK", 0x05);
	}

	g_msg->header->if_des_type	  = AK_APP_TYPE_IF;
	g_msg->header->if_sig		  = 0xFFFFFFFF;
	g_msg->header->if_src_task_id = 0xFFFFFFFF;
	g_msg->header->if_des_task_id = 0xFFFFFFFF;

	g_msg->header->type	   = COMMON_MSG_TYPE;
	g_msg->header->len	   = 0;
	g_msg->header->payload = NULL;

	AK_MSG_DBG("[MSG] get msg:%p\theader:%p\n", g_msg, g_msg->header);
	return g_msg;
}

ak_msg_t *ak_memcpy_msg(ak_msg_t *src) {
	ak_msg_t *ret_msg = AK_MSG_NULL;

	if (src == AK_MSG_NULL) {
		FATAL("AK", 0x06);
	}

	switch (get_msg_type(src)) {
	case PURE_MSG_TYPE: {
		/* get pure message */
		ret_msg = get_pure_msg();

		/* copy message header */
		memcpy(ret_msg->header, src->header, sizeof(header_t));
	} break;

	case DYNAMIC_MSG_TYPE: {
		/* get dynamic message */
		ret_msg = get_dynamic_msg();

		/* copy message header */
		memcpy(ret_msg->header, src->header, sizeof(header_t));

		/* copy dynamic payload */
		set_data_dynamic_msg(ret_msg, (uint8_t *)src->header->payload, src->header->len);
	} break;

	case COMMON_MSG_TYPE: {
		/* get common message */
		ret_msg = get_common_msg();

		/* copy message header */
		memcpy(ret_msg->header, src->header, sizeof(header_t));

		/* copy common payload */
		set_data_common_msg(ret_msg, (uint8_t *)src->header->payload, src->header->len);
	} break;

	default:
		break;
	}

	return ret_msg;
}

void ak_msg_free(ak_msg_t *msg) {
	if (msg != NULL) {
		q_msg_free(msg);
	}
	else {
		FATAL("AK", 0x07);
	}
}

void set_msg_sig(ak_msg_t *msg, uint32_t sig) {
	if (msg != NULL) {
		msg->header->sig = sig;
	}
	else {
		FATAL("AK", 0x08);
	}
}

void set_msg_root_task_id(ak_msg_t *msg, uint32_t root_task_id) {
	if (msg != NULL) {
		msg->header->root_task_id = root_task_id;
	}
	else {
		FATAL("AK", 0x1E);
	}
}

void set_msg_des_task_id(ak_msg_t *msg, uint32_t des_task_id) {
	if (msg != NULL) {
		msg->header->des_task_id = des_task_id;
	}
	else {
		FATAL("AK", 0x09);
	}
}

void set_msg_src_task_id(ak_msg_t *msg, uint32_t src_task_id) {
	if (msg != NULL) {
		msg->header->src_task_id = src_task_id;
	}
	else {
		FATAL("AK", 0x09);
	}
}

void set_if_des_task_id(ak_msg_t *msg, uint32_t id) {
	if (msg != NULL) {
		msg->header->if_des_task_id = id;
	}
	else {
		FATAL("AK", 0x0A);
	}
}

void set_if_src_task_id(ak_msg_t *msg, uint32_t id) {
	if (msg != NULL) {
		msg->header->if_src_task_id = id;
	}
	else {
		FATAL("AK", 0x0A);
	}
}

void set_if_des_type(ak_msg_t *msg, uint32_t type) {
	if (msg != NULL) {
		msg->header->if_des_type = type;
	}
	else {
		FATAL("AK", 0x0B);
	}
}

void set_if_src_type(ak_msg_t *msg, uint32_t type) {
	if (msg != NULL) {
		msg->header->if_src_type = type;
	}
	else {
		FATAL("AK", 0x0B);
	}
}

void set_if_sig(ak_msg_t *msg, uint32_t sig) {
	if (msg != NULL) {
		msg->header->if_sig = sig;
	}
	else {
		FATAL("AK", 0x0B);
	}
}

void set_if_data_common_msg(ak_msg_t *msg, uint8_t *data, uint32_t len) {
	set_data_common_msg(msg, data, len);
}

void set_if_data_dynamic_msg(ak_msg_t *msg, uint8_t *data, uint32_t len) {
	set_data_dynamic_msg(msg, data, len);
}

void set_data_common_msg(ak_msg_t *msg, uint8_t *data, uint32_t len) {
	if (msg != NULL) {
		if (msg->header->type == COMMON_MSG_TYPE) {
			msg->header->payload = (uint8_t *)malloc((size_t)len);
			if (msg->header->payload == NULL) {
				FATAL("AK", 0x0D);
			}
			else {
				if (len > AK_COMMON_MSG_DATA_SIZE) {
					FATAL("AK", 0x0E);
				}
				else {
					msg->header->len = len;
					memcpy(msg->header->payload, data, len);
					AK_MSG_DBG("[MSG] set payload:%p\n", msg->header->payload);
					AK_MSG_DBG("[MSG] set payload len:%d\n", msg->header->len);
				}
			}
		}
		else {
			FATAL("AK", 0x05);
		}
	}
	else {
		FATAL("AK", 0x03);
	}
}

void get_data_common_msg(ak_msg_t *msg, uint8_t *data, uint32_t len) {
	if (msg != NULL) {
		if (msg->header->type == COMMON_MSG_TYPE) {
			if (msg->header->payload == NULL || msg->header->len < len) {
				FATAL("AK", 0x0F);
			}
			else {
				memcpy(data, msg->header->payload, len);
				AK_MSG_DBG("[MSG] get payload:%p\n", msg->header->payload);
			}
		}
		else {
			FATAL("AK", 0x10);
		}
	}
	else {
		FATAL("AK", 0x11);
	}
}

uint8_t *get_data_common_msg(ak_msg_t *msg) {
	uint8_t *ret = NULL;
	if (msg != NULL) {
		if (msg->header->type == COMMON_MSG_TYPE) {
			if (msg->header->payload == NULL) {
				FATAL("AK", 0x0F);
			}
			else {
				ret = (uint8_t *)msg->header->payload;
				AK_MSG_DBG("[MSG] get payload:%p\n", msg->header->payload);
			}
		}
		else {
			FATAL("AK", 0x10);
		}
	}
	else {
		FATAL("AK", 0x11);
	}
	return ret;
}

uint8_t get_data_len_common_msg(ak_msg_t *msg) {
	uint8_t ret = 0;
	if (msg != NULL) {
		if (msg->header->type == COMMON_MSG_TYPE) {
			ret = msg->header->len;
		}
		else {
			FATAL("AK", 0x12);
		}
	}
	else {
		FATAL("AK", 0x13);
	}
	return ret;
}

void set_data_dynamic_msg(ak_msg_t *msg, uint8_t *data, uint32_t len) {
	if (msg != NULL) {
		if (msg->header->type == DYNAMIC_MSG_TYPE) {
			msg->header->payload = (uint8_t *)malloc((size_t)len);
			if (msg->header->payload == NULL) {
				FATAL("AK", 0x14);
			}
			else {
				msg->header->len = len;
				memcpy(msg->header->payload, data, len);
				AK_MSG_DBG("[MSG] set payload:%p\n", msg->header->payload);
				AK_MSG_DBG("[MSG] set payload len:%d\n", msg->header->len);
			}
		}
		else {
			FATAL("AK", 0x15);
		}
	}
	else {
		FATAL("AK", 0x16);
	}
}

void get_data_dynamic_msg(ak_msg_t *msg, uint8_t *data, uint32_t len) {
	if (msg != NULL) {
		if (msg->header->type == DYNAMIC_MSG_TYPE) {
			if (msg->header->payload == NULL || msg->header->len < len) {
				FATAL("AK", 0x17);
			}
			else {
				memcpy(data, msg->header->payload, len);
				AK_MSG_DBG("[MSG] get payload:%p\n", msg->header->payload);
			}
		}
		else {
			FATAL("AK", 0x18);
		}
	}
	else {
		FATAL("AK", 0x19);
	}
}

uint32_t get_data_len_dynamic_msg(ak_msg_t *msg) {
	uint32_t ret = 0;
	if (msg != NULL) {
		if (msg->header->type == DYNAMIC_MSG_TYPE) {
			ret = msg->header->len;
		}
		else {
			FATAL("AK", 0x22);
		}
	}
	else {
		FATAL("AK", 0x23);
	}
	return ret;
}

void task_post(uint32_t task_dst_id, ak_msg_t *msg) {
	if (task_dst_id >= ak_thread_table_len) {
		FATAL("AK", 0x1A);
	}

	if (msg != NULL) {
		pthread_mutex_lock(&(task_list[task_dst_id].mt_mailbox_cond));

		msg->header->des_task_id = task_dst_id;
		q_msg_t *q_msg			 = task_list[task_dst_id].mailbox;
		q_msg_put(q_msg, msg);

		pthread_cond_signal(&(task_list[task_dst_id].mailbox_cond));

		pthread_mutex_unlock(&(task_list[task_dst_id].mt_mailbox_cond));
	}
	else {
		FATAL("AK", 0x1B);
	}
}

void task_post_pure_msg(uint32_t task_src_id, uint32_t task_dst_id, uint32_t sig) {
	ak_msg_t *s_msg = get_pure_msg();
	set_msg_sig(s_msg, sig);
	set_msg_src_task_id(s_msg, task_src_id);
	task_post(task_dst_id, s_msg);
}

void task_post_pure_msg(uint32_t task_dst_id, uint32_t sig) {
	task_post_pure_msg(task_dst_id, task_dst_id, sig);
}

void task_post_common_msg(uint32_t task_src_id, uint32_t task_dst_id, uint32_t sig, uint8_t *data, uint32_t len) {
	ak_msg_t *s_msg = get_common_msg();
	set_msg_sig(s_msg, sig);
	set_data_common_msg(s_msg, data, len);
	set_msg_src_task_id(s_msg, task_src_id);
	task_post(task_dst_id, s_msg);
}

void task_post_common_msg(uint32_t task_dst_id, uint32_t sig, uint8_t *data, uint32_t len) {
	task_post_common_msg(task_dst_id, task_dst_id, sig, data, len);
}

void task_post_dynamic_msg(uint32_t task_src_id, uint32_t task_dst_id, uint32_t sig, uint8_t *data, uint32_t len) {
	ak_msg_t *s_msg = get_dynamic_msg();
	set_msg_sig(s_msg, sig);
	set_data_dynamic_msg(s_msg, data, len);
	set_msg_src_task_id(s_msg, task_src_id);
	task_post(task_dst_id, s_msg);
}

void task_post_dynamic_msg(uint32_t task_dst_id, uint32_t sig, uint8_t *data, uint32_t len) {
	task_post_dynamic_msg(task_dst_id, task_dst_id, sig, data, len);
}

void task_root_post_pure_msg(uint32_t task_root_id, uint32_t task_src_id, uint32_t task_dst_id, uint32_t sig) {
	ak_msg_t *s_msg = get_pure_msg();
	set_msg_sig(s_msg, sig);
	set_msg_root_task_id(s_msg, task_root_id);
	set_msg_src_task_id(s_msg, task_src_id);
	task_post(task_dst_id, s_msg);
}

void task_root_post_pure_msg(uint32_t task_root_id, uint32_t task_dst_id, uint32_t sig) {
	task_root_post_pure_msg(task_root_id, task_dst_id, task_dst_id, sig);
}

void task_root_post_common_msg(uint32_t task_root_id, uint32_t task_src_id, uint32_t task_dst_id, uint32_t sig, uint8_t *data, uint32_t len) {
	ak_msg_t *s_msg = get_common_msg();
	set_msg_sig(s_msg, sig);
	set_data_common_msg(s_msg, data, len);
	set_msg_root_task_id(s_msg, task_root_id);
	set_msg_src_task_id(s_msg, task_src_id);
	task_post(task_dst_id, s_msg);
}

void task_root_post_common_msg(uint32_t task_root_id, uint32_t task_dst_id, uint32_t sig, uint8_t *data, uint32_t len) {
	task_root_post_common_msg(task_root_id, task_dst_id, task_dst_id, sig, data, len);
}

void task_root_post_dynamic_msg(uint32_t task_root_id, uint32_t task_src_id, uint32_t task_dst_id, uint32_t sig, uint8_t *data, uint32_t len) {
	ak_msg_t *s_msg = get_dynamic_msg();
	set_msg_sig(s_msg, sig);
	set_data_dynamic_msg(s_msg, data, len);
	set_msg_root_task_id(s_msg, task_root_id);
	set_msg_src_task_id(s_msg, task_src_id);
	task_post(task_dst_id, s_msg);
}

void task_root_post_dynamic_msg(uint32_t task_root_id, uint32_t task_dst_id, uint32_t sig, uint8_t *data, uint32_t len) {
	task_root_post_dynamic_msg(task_root_id, task_dst_id, task_dst_id, sig, data, len);
}

ak_msg_t *ak_msg_rev(uint32_t des_task_id) {
	ak_msg_t *ret_msg = AK_MSG_NULL;

	if (des_task_id >= AK_TASK_LIST_LEN) {
		FATAL("AK", 0x1D);
	}

	pthread_mutex_lock(&(task_list[des_task_id].mt_mailbox_cond));

	q_msg_t *q_msg = task_list[des_task_id].mailbox;

	if (q_msg->len == 0) {
		pthread_cond_wait(&(task_list[des_task_id].mailbox_cond), &(task_list[des_task_id].mt_mailbox_cond));
	}

	if (q_msg_available(q_msg)) {
		ret_msg = q_msg_get(q_msg);
	}

	pthread_mutex_unlock(&(task_list[des_task_id].mt_mailbox_cond));

	return ret_msg;
}

uint32_t get_msg_type(ak_msg_t *msg) {
	return msg->header->type;
}

int get_task_id() {
	pthread_t current_thread_id = pthread_self();
	for (int i = 0; i < AK_TASK_LIST_LEN; i++) {
		if (task_list[i].pthread == current_thread_id) {
			return task_list[i].id;
		}
	}
	return -1;
}
