#include "task_hello.h"
#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "timer.h" 

// Shared variables for LCD rendering (scr_idle.cpp)
game_state_t g_state = GAME_STATE_MENU;
game_diff_t g_difficulty = DIFF_NORMAL;

int8_t frog_x = 59;
int8_t frog_y = 52;
int8_t checkpoint_x = 59;
int8_t checkpoint_y = 52;
uint8_t player_lives = 2;
uint16_t game_score = 0;

// Coordinates and speeds of 3 car lanes
int16_t car1_x = 10;   int16_t car1_y = 44;  int8_t car1_speed = 2;
int16_t car2_x = 100;  int16_t car2_y = 28;  int8_t car2_speed = -3; // Moving in opposite direction
int16_t car3_x = 50;   int16_t car3_y = 16;  int8_t car3_speed = 4;

// AABB (Axis-Aligned Bounding Box) Collision Detection
bool check_collision(int16_t x1, int16_t y1, int16_t w1, int16_t h1,
                     int16_t x2, int16_t y2, int16_t w2, int16_t h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void task_hello(ak_msg_t* msg) {
    switch (msg->sig) {
    
    // 1. UP BUTTON (MOVE FORWARD / SELECT IN MENU)
    case AC_TASK_HELLO_PRINT: {
        if (g_state == GAME_STATE_MENU) {
            // Enter game
            g_state = GAME_STATE_PLAYING;
            frog_x = 59; frog_y = 52;
            checkpoint_x = 59; checkpoint_y = 52;
            game_score = 0;

            if (g_difficulty == DIFF_EASY) player_lives = 3;
            else if (g_difficulty == DIFF_NORMAL) player_lives = 2;
            else player_lives = 1;

            // Switch LCD to playing screen
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE);
            
            // Start game tick timer (50ms interval)
            timer_set(AC_TASK_HELLO_ID, AC_TASK_HELLO_TICK, GAME_TICK_INTERVAL_MS, TIMER_PERIODIC);
            
            APP_DBG("[GAME] Game Start! Difficulty: %d, Lives: %d\n", g_difficulty, player_lives);
        }
        else if (g_state == GAME_STATE_PLAYING) {
            if (frog_y > 15) {
                frog_y -= 6; // Jump forward
                
                // Reached Checkpoint (Safe zone at Y = 36)
                if (frog_y <= 36 && checkpoint_y > 36) {
                    checkpoint_x = frog_x;
                    checkpoint_y = frog_y;
                    APP_DBG("[GAME] Checkpoint reached!\n");
                }

                task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
            }
        }
        else if (g_state == GAME_STATE_GAMEOVER) {
            g_state = GAME_STATE_MENU;
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_LOGO); // Return to welcome screen
        }
        break;
    }

    // 2. DOWN BUTTON (MOVE LEFT / DECREASE DIFFICULTY)
    case AC_TASK_HELLO_LEFT: {
        if (g_state == GAME_STATE_MENU) {
            if (g_difficulty == DIFF_NORMAL) g_difficulty = DIFF_EASY;
            else if (g_difficulty == DIFF_HARD) g_difficulty = DIFF_NORMAL;
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
        }
        else if (g_state == GAME_STATE_PLAYING) {
            if (frog_x > 4) {
                frog_x -= 6; // Jump left
                task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
            }
        }
        break;
    }

    // 3. MODE BUTTON (MOVE RIGHT / INCREASE DIFFICULTY)
    case AC_TASK_HELLO_RIGHT: {
        if (g_state == GAME_STATE_MENU) {
            if (g_difficulty == DIFF_EASY) g_difficulty = DIFF_NORMAL;
            else if (g_difficulty == DIFF_NORMAL) g_difficulty = DIFF_HARD;
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
        }
        else if (g_state == GAME_STATE_PLAYING) {
            if (frog_x < 114) {
                frog_x += 6; // Jump right
                task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
            }
        }
        break;
    }

    // 4. GAME TICK (GAME LOOP) - Runs every 50ms
    case AC_TASK_HELLO_TICK: {
        if (g_state == GAME_STATE_PLAYING) {
            // Multiply car speed based on difficulty
            int8_t speed_mult = (g_difficulty == DIFF_HARD) ? 2 : 1;

            // Move 3 car lanes
            car1_x += car1_speed * speed_mult;
            if (car1_x > 128) car1_x = -15;

            car2_x += car2_speed * speed_mult;
            if (car2_x < -20) car2_x = 128;

            car3_x += car3_speed * speed_mult;
            if (car3_x > 128) car3_x = -15;

            // CHECK COLLISION (Frog 10x8, Car 15x6 pixels)
            bool collision = false;
            collision |= check_collision(frog_x, frog_y, 10, 8, car1_x, car1_y, 15, 6);
            collision |= check_collision(frog_x, frog_y, 10, 8, car2_x, car2_y, 20, 6); // Car 2 is longer
            collision |= check_collision(frog_x, frog_y, 10, 8, car3_x, car3_y, 12, 6);

            if (collision) {
                APP_DBG("[GAME] BEEP! Frog got hit!\n");
                player_lives--;
                if (player_lives > 0) {
                    // Respawn at checkpoint
                    frog_x = checkpoint_x;
                    frog_y = checkpoint_y;
                } else {
                    // Out of lives -> GAME OVER
                    g_state = GAME_STATE_GAMEOVER;
                    timer_remove_attr(AC_TASK_HELLO_ID, AC_TASK_HELLO_TICK); // Stop game tick timer
                    APP_DBG("[GAME] GAME OVER!\n");
                }
            }

            // CHECK WIN CONDITION (Frog reached GOAL Y <= 15)
            if (frog_y <= 15) {
                game_score += 100;
                APP_DBG("[GAME] Win! Score: %d\n", game_score);
                // Reset frog to start position but keep score
                frog_x = 59;
                frog_y = 52;
                checkpoint_x = 59;
                checkpoint_y = 52;
            }

            // Request full screen render
            task_post_pure_msg(AC_TASK_DISPLAY_ID, AC_DISPLAY_RENDER_SCREEN);
        }
        break;
    }

    default:
        break;
    }
}
