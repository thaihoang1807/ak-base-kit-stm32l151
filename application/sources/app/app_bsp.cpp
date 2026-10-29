#include "button.h"
#include "sys_dbg.h"
#include "app.h"
#include "app_bsp.h"
#include "task_list.h"
#include "arcade.h"

button_t btn_mode, btn_up, btn_down;

void btn_mode_callback(void* b) {
    button_t* button = (button_t*)b;

    if (button->state == BUTTON_SW_STATE_PRESSED) {
        // Nhan MODE ngay khi nhan xuong.
        task_post_pure_msg(
            AC_TASK_HELLO_ID,
            AC_TASK_HELLO_RIGHT
        );
    } else if (
        button->state == BUTTON_SW_STATE_LONG_PRESSED
    ) {
        task_post_pure_msg(
            AC_TASK_HELLO_ID,
            ARCADE_EXIT_SIG
        );
    }
}

void btn_up_callback(void* b) {
    button_t* button = (button_t*)b;

    if (button->state == BUTTON_SW_STATE_PRESSED) {
        task_post_pure_msg(
            AC_TASK_HELLO_ID,
            AC_TASK_HELLO_PRINT
        );
    }
}

void btn_down_callback(void* b) {
    button_t* button = (button_t*)b;

    if (button->state == BUTTON_SW_STATE_PRESSED) {
        task_post_pure_msg(
            AC_TASK_HELLO_ID,
            AC_TASK_HELLO_LEFT
        );
    }
}
