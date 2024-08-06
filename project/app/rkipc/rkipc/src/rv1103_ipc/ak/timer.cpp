/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   12/01/2017
 * @brief:  timer
 ******************************************************************************
 **/
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <queue>

#include "ak.h"
#include "ak_dbg.h"
#include "timer.h"

#include "sys_dbg.h"

#include "task_list.h"

/* internal define */
#define CLOCKID			   CLOCK_REALTIME
#define TIMER_INTERVAL_SIG SIGUSR1

#define TIMER_ADD_SIG 0x01

/* declare internal struct */
typedef struct {
	uint32_t root_task_id;
	uint32_t des_task_id;
	uint32_t sig;

	int32_t period;
	int32_t counter;
} timer_data_t;

typedef struct timer_msg_t {
	timer_msg_t *next;
	timer_msg_t *prev;
	timer_data_t data;
} timer_msg_t;

typedef struct {
	pthread_mutex_t mt;
	timer_msg_t *head;
	timer_msg_t *tail;
} timer_service_t;

/* external variable module */
q_msg_t timer_mailbox;

/* internal variable */
static timer_service_t timer_service;
static pthread_mutex_t mt_timer_service;

static timer_t timer_id;

/* internal function */
static uint32_t timer_service_remove_node(uint32_t dst, uint32_t sig);

/* timer stick counter */
static volatile uint32_t timer_stick_counter;

void timer_handler(sigval_t) {
	timer_stick_counter += AK_TIMER_UNIT;

	pthread_mutex_lock(&mt_timer_service);

	timer_msg_t *remove_node_mask = NULL;
	timer_msg_t *timer_node_temp  = timer_service.tail;

	while (timer_node_temp != NULL) {
		timer_node_temp->data.counter -= AK_TIMER_UNIT;
		if (timer_node_temp->data.counter <= 0) {
			if (timer_node_temp->data.root_task_id != 0xFFFFFFFF) {
				task_root_post_pure_msg(timer_node_temp->data.root_task_id, AK_TASK_TIMER_ID, timer_node_temp->data.des_task_id, timer_node_temp->data.sig);
			}
			else
				task_post_pure_msg(AK_TASK_TIMER_ID, timer_node_temp->data.des_task_id, timer_node_temp->data.sig);
			AK_TIMER_DBG("[TIMER][timer_handler] des_task_id:%d sig:%d\n", timer_node_temp->data.des_task_id, timer_node_temp->data.sig);

			if (timer_node_temp->data.period != 0) {
				timer_node_temp->data.counter = timer_node_temp->data.period;
			}
			else {
				remove_node_mask = timer_node_temp;
			}
		}

		/* check next node */
		timer_node_temp = timer_node_temp->next;

		/* remove node */
		if (remove_node_mask != NULL) {
			timer_service_remove_node(remove_node_mask->data.des_task_id, remove_node_mask->data.sig);
			remove_node_mask = NULL;
		}
	}

	pthread_mutex_unlock(&mt_timer_service);
}

void *timer_entry(void *) {
	timer_service.head = NULL;
	timer_service.tail = NULL;

	/* configure timer */
	struct sigevent sev;
	struct itimerspec its;

	/* create the timer */
	sev.sigev_notify			= SIGEV_THREAD;
	sev.sigev_signo				= TIMER_INTERVAL_SIG;
	sev.sigev_value.sival_ptr	= &timer_id;
	sev.sigev_notify_attributes = NULL;
	sev.sigev_notify_function	= timer_handler;
	timer_create(CLOCKID, &sev, &timer_id);

	/* start the timer */
	its.it_value.tv_sec		= 0;
	its.it_value.tv_nsec	= AK_TIMER_UNIT * 1000000;
	its.it_interval.tv_sec	= its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;
	timer_settime(timer_id, 0, &its, NULL);

	wait_all_tasks_started();

	return (void *)0;
}
uint32_t timer_set(uint32_t root_task_id, uint32_t des_task_id, uint32_t sig, uint32_t duty, timer_type_t timer_type) {
	AK_TIMER_DBG("[TIMER][timer_set] des_task_id:%d sig:%d duty:%d timer_type:%d\n", des_task_id, sig, duty, timer_type);

	if (duty < AK_TIMER_UNIT) {
		AK_TIMER_WRN("[TIMER][timer_set] ( duty < AK_TIMER_UNIT) des_task_id:%d sig:%d duty:%d timer_type:%d\n", des_task_id, sig, duty, timer_type);
		FATAL("TIMER", 0x03);
	}

	/* add node to timer list */
	pthread_mutex_lock(&mt_timer_service);

	timer_msg_t *timer_node_temp = timer_service.tail;

	while (timer_node_temp != NULL) {
		if (timer_node_temp->data.des_task_id == des_task_id && timer_node_temp->data.sig == sig) {
			timer_node_temp->data.counter = duty;
			AK_TIMER_DBG("[TIMER] clear counter\n");

			pthread_mutex_unlock(&mt_timer_service);

			return AK_RET_OK;
		}
		else {
			timer_node_temp = timer_node_temp->next;
		}
	}

	timer_msg_t *timer_new = (timer_msg_t *)malloc(sizeof(timer_msg_t));

	if (timer_new == NULL) {
		FATAL("TIMER", 0x02);
	}

	/* update timer node infomation */
	timer_new->data.root_task_id = root_task_id;
	timer_new->data.des_task_id	 = des_task_id;
	timer_new->data.sig			 = sig;
	timer_new->data.counter		 = duty;

	switch (timer_type) {
	case TIMER_ONE_SHOT:
		timer_new->data.period = (int32_t)0;
		break;

	case TIMER_PERIODIC:
		timer_new->data.period = (int32_t)duty;
		break;

	default: {
		FATAL("TIMER", 0x01);
	} break;
	}

	if (timer_service.tail != NULL) {
		timer_new->prev = NULL;
		timer_new->next = timer_service.tail;

		timer_service.tail->prev = timer_new;
		timer_service.tail		 = timer_new;
	}
	else {
		timer_new->next	   = NULL;
		timer_new->prev	   = NULL;
		timer_service.tail = timer_service.head = timer_new;
	}

	pthread_mutex_unlock(&mt_timer_service);

	return AK_RET_OK;
}

uint32_t timer_set(uint32_t des_task_id, uint32_t sig, uint32_t duty, timer_type_t timer_type) {
	AK_TIMER_DBG("[TIMER][timer_set] des_task_id:%d sig:%d duty:%d timer_type:%d\n", des_task_id, sig, duty, timer_type);

	if (duty < AK_TIMER_UNIT) {
		AK_TIMER_WRN("[TIMER][timer_set] ( duty < AK_TIMER_UNIT) des_task_id:%d sig:%d duty:%d timer_type:%d\n", des_task_id, sig, duty, timer_type);
		FATAL("TIMER", 0x03);
	}

	/* add node to timer list */
	pthread_mutex_lock(&mt_timer_service);

	timer_msg_t *timer_node_temp = timer_service.tail;

	while (timer_node_temp != NULL) {
		if (timer_node_temp->data.des_task_id == des_task_id && timer_node_temp->data.sig == sig) {
			timer_node_temp->data.counter = duty;
			AK_TIMER_DBG("[TIMER] clear counter\n");

			pthread_mutex_unlock(&mt_timer_service);

			return AK_RET_OK;
		}
		else {
			timer_node_temp = timer_node_temp->next;
		}
	}

	timer_msg_t *timer_new = (timer_msg_t *)malloc(sizeof(timer_msg_t));

	if (timer_new == NULL) {
		FATAL("TIMER", 0x02);
	}

	/* update timer node infomation */
	timer_new->data.root_task_id = 0xFFFFFFFF;
	timer_new->data.des_task_id	 = des_task_id;
	timer_new->data.sig			 = sig;
	timer_new->data.counter		 = duty;

	switch (timer_type) {
	case TIMER_ONE_SHOT:
		timer_new->data.period = (int32_t)0;
		break;

	case TIMER_PERIODIC:
		timer_new->data.period = (int32_t)duty;
		break;

	default: {
		FATAL("TIMER", 0x01);
	} break;
	}

	if (timer_service.tail != NULL) {
		timer_new->prev = NULL;
		timer_new->next = timer_service.tail;

		timer_service.tail->prev = timer_new;
		timer_service.tail		 = timer_new;
	}
	else {
		timer_new->next	   = NULL;
		timer_new->prev	   = NULL;
		timer_service.tail = timer_service.head = timer_new;
	}

	pthread_mutex_unlock(&mt_timer_service);

	return AK_RET_OK;
}

uint32_t timer_remove_attr(uint32_t des_task_id, uint32_t sig) {
	uint32_t ret;
	pthread_mutex_lock(&mt_timer_service);
	ret = timer_service_remove_node(des_task_id, sig);
	pthread_mutex_unlock(&mt_timer_service);
	return ret;
}

uint32_t timer_service_remove_node(uint32_t dst, uint32_t sig) {
	timer_msg_t *timer_node_temp = timer_service.tail;

	while (timer_node_temp != NULL) {
		if (timer_node_temp->data.des_task_id == dst && timer_node_temp->data.sig == sig) {
			/* remove timer node */
			/* last node */
			if (timer_node_temp->next == NULL && timer_node_temp->prev == NULL) {
				timer_service.head = timer_service.tail = NULL;
			}
			/* middle node */
			else if (timer_node_temp->next != NULL && timer_node_temp->prev != NULL) {
				timer_node_temp->next->prev = timer_node_temp->prev;
				timer_node_temp->prev->next = timer_node_temp->next;
			}
			/* tail node */
			else if (timer_node_temp->prev == NULL) {
				timer_service.tail		 = timer_service.tail->next;
				timer_service.tail->prev = NULL;
			}
			/* head node */
			else if (timer_node_temp->next == NULL) {
				timer_service.head		 = timer_service.head->prev;
				timer_service.head->next = NULL;
			}
			else {
				FATAL("TIMER", 0xAA);
			}

			free(timer_node_temp);

			return AK_RET_OK;
		}
		else {
			timer_node_temp = timer_node_temp->next;
		}
	}

	return AK_RET_NG;
}

uint32_t timer_stick_get() {
	return timer_stick_counter;
}
