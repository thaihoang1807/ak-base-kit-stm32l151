#ifndef __TASK_HELLO_H__
#define __TASK_HELLO_H__

#include "ak.h"
#include "message.h"

// Định nghĩa các trạng thái của Game
typedef enum {
    GAME_STATE_MENU,      // Màn hình chọn độ khó
    GAME_STATE_PLAYING,   // Đang chơi
    GAME_STATE_DEAD,      // Chết (về checkpoint)
    GAME_STATE_GAMEOVER   // Thua cuộc hoàn toàn
} game_state_t;

// Định nghĩa mức độ khó
typedef enum {
    DIFF_EASY,
    DIFF_NORMAL,
    DIFF_HARD
} game_diff_t;

extern void task_hello(ak_msg_t* msg);

#endif // __TASK_HELLO_H__