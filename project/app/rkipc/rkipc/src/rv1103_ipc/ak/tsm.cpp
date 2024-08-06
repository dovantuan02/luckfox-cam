/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 ******************************************************************************
 **/

#include "ak.h"
#include "message.h"
#include "sys_dbg.h"

void tsm_init(tsm_tbl_t *tsm_tbl, tsm_t **tbl, tsm_state_t state) {
	if (tsm_tbl == (tsm_tbl_t *)0) {
		FATAL("TSM", 0x01);
	}

	tsm_tbl->state = state;
	tsm_tbl->table = tbl;
}

void tsm_dispatch(tsm_tbl_t *tsm_tbl, ak_msg_t *msg) {
	tsm_t *respective_table = tsm_tbl->table[tsm_tbl->state];

	/* search tsm state respective */
	while ((respective_table->sig != msg->header->sig) && (respective_table->sig != TSM_EOT_SIGNAL)) {
		respective_table++;
	}

	/* update next state */
	tsm_tbl->state = respective_table->next_state;

	/* signal state changed */
	if (tsm_tbl->on_state != NULL) {
		tsm_tbl->on_state(tsm_tbl->state);
	}

	if (respective_table->tsm_func != TSM_FUNCTION_NULL) {
		respective_table->tsm_func(msg);
	}
}

void tsm_tran(tsm_tbl_t *tsm_tbl, tsm_state_t state) {
	tsm_tbl->state = state;

	/* signal state changed */
	if (tsm_tbl->on_state != NULL) {
		tsm_tbl->on_state(tsm_tbl->state);
	}
}
