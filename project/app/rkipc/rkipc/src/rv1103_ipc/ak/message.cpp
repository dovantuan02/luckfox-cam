/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   12/01/2017
 * @brief: mailbox
 ******************************************************************************
 **/
#include <stdlib.h>

#include "ak_dbg.h"
#include "message.h"

void q_msg_init(q_msg_t *q_msg) {
	q_msg->head = NULL;
	q_msg->tail = NULL;
	q_msg->len	= 0;
}

void q_msg_put(q_msg_t *q_msg, ak_msg_t *msg) {
	pthread_mutex_lock(&(q_msg->mt));

	if (q_msg->tail != NULL) {
		msg->prev = NULL;
		msg->next = q_msg->tail;

		q_msg->tail->prev = msg;
		q_msg->tail		  = msg;
	}
	else {
		msg->next	= NULL;
		msg->prev	= NULL;
		q_msg->tail = q_msg->head = msg;
	}

	q_msg->len++;

	pthread_mutex_unlock(&(q_msg->mt));
}

ak_msg_t *q_msg_get(q_msg_t *q_msg) {
	ak_msg_t *temp_head;
	ak_msg_t *msg = NULL;

	pthread_mutex_lock(&(q_msg->mt));

	if (q_msg->head != NULL) {
		msg = q_msg->head;
		if (q_msg->head == q_msg->tail) {
			q_msg->head = q_msg->tail = NULL;
		}
		else {
			temp_head				= q_msg->head->prev;
			q_msg->head->prev->next = NULL;
			q_msg->head->prev		= NULL;

			q_msg->head = temp_head;
		}
	}

	q_msg->len--;

	pthread_mutex_unlock(&(q_msg->mt));

	return msg;
}

void q_msg_free(ak_msg_t *msg) {
	if (msg != NULL) {
		if (msg->header != NULL) {
			if (msg->header->payload != NULL) {
				free(msg->header->payload);
				AK_MSG_DBG("[MSG] free payload:%p\n", msg->header->payload);
			}
			free(msg->header);
			AK_MSG_DBG("[MSG] free header:%p\n", msg->header);
		}

		free(msg);
		AK_MSG_DBG("[MSG] free msg:%p\n", msg);
	}
}

uint32_t q_msg_len(q_msg_t *q_msg) {
	uint32_t counter = 0;

	pthread_mutex_lock(&(q_msg->mt));

	ak_msg_t *qry_msg = q_msg->tail;

	while (qry_msg != NULL) {
		counter++;
		qry_msg = qry_msg->next;
	}

	pthread_mutex_unlock(&(q_msg->mt));

	return counter;
}

bool q_msg_available(q_msg_t *q_msg) {
	bool ret;
	pthread_mutex_lock(&(q_msg->mt));
	if (q_msg->head != NULL && q_msg->tail != NULL) {
		ret = true;
	}
	else {
		ret = false;
	}
	pthread_mutex_unlock(&(q_msg->mt));
	return ret;
}
