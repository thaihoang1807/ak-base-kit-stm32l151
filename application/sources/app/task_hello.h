#ifndef __TASK_HELLO_H__
#define __TASK_HELLO_H__

#include "ak.h"
#include "message.h"

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_DEAD,
    GAME_STATE_GAMEOVER
} game_state_t;

typedef enum {
    DIFF_EASY,
    DIFF_NORMAL,
    DIFF_HARD
} game_diff_t;

typedef enum {
    SCENE_ROAD,
    SCENE_RIVER,
    SCENE_FOREST
} game_scene_t;

extern game_state_t g_state;
extern game_diff_t g_difficulty;
extern game_scene_t g_scene;

extern int8_t frog_x, frog_y;

extern int16_t car1_x, car1_y;
extern int16_t car2_x, car2_y;
extern int16_t car3_x, car3_y;

extern int16_t log_x[3];

extern uint8_t player_lives;
extern uint8_t game_level;
extern uint8_t transition_ticks;
extern uint8_t respawn_ticks;

extern uint16_t game_score;
extern uint16_t best_score;
extern uint16_t time_ticks_left;

extern int16_t coin_x, coin_y;
extern int16_t heart_x, heart_y;
extern bool coin_active;
extern bool heart_active;

extern uint8_t game_time_seconds();
extern int16_t game_log_width();
extern void task_hello(ak_msg_t* msg);

#endif
