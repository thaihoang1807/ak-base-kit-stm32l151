#include "button.h"
#include "sys_dbg.h"
#include "app.h"
#include "app_bsp.h"
#include "task_list.h"

button_t btn_mode; button_t btn_up; button_t btn_down;

void btn_mode_callback(void* b) {
	button_t* me_b = (button_t*)b;
	if (me_b->state == BUTTON_SW_STATE_PRESSED) {
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_MODE_PRESSED);
		task_post_pure_msg(AC_TASK_HELLO_ID, AC_TASK_HELLO_RIGHT);
	}
}

void btn_up_callback(void* b) {
	button_t* me_b = (button_t*)b;
	if (me_b->state == BUTTON_SW_STATE_PRESSED) {
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_UP_PRESSED);
		task_post_pure_msg(AC_TASK_HELLO_ID, AC_TASK_HELLO_PRINT);
	}
}

void btn_down_callback(void* b) {
	button_t* me_b = (button_t*)b;
	if (me_b->state == BUTTON_SW_STATE_PRESSED) {
		task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_BUTON_DOWN_PRESSED);
		task_post_pure_msg(AC_TASK_HELLO_ID, AC_TASK_HELLO_LEFT);
	}
}
