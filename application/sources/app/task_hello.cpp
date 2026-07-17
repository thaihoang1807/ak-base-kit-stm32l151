#include "task_hello.h"
#include "app.h"
#include "app_dbg.h"
#include "task_list.h"

// Biến quản lý trạng thái game
static game_state_t g_state = GAME_STATE_MENU;
static game_diff_t g_difficulty = DIFF_NORMAL;

static int8_t frog_x = 30; // Tọa độ X của ếch (trái/phải)
static int8_t frog_y = 60; // Tọa độ Y của ếch (tiến lên)
static int8_t checkpoint_x = 30;
static int8_t checkpoint_y = 60;
static uint8_t player_lives = 2;

void task_hello(ak_msg_t* msg) {
    switch (msg->sig) {
    case AC_TASK_HELLO_PRINT: {
        // Tận dụng lại signal bấm nút UP từ app_bsp.cpp gửi qua
        
        if (g_state == GAME_STATE_MENU) {
            // Nếu đang ở Menu, nút UP dùng để CHỌN độ khó và VÀO GAME
            g_state = GAME_STATE_PLAYING;
            
            // Cấu hình mạng sống theo độ khó
            if (g_difficulty == DIFF_EASY) player_lives = 3;
            else if (g_difficulty == DIFF_NORMAL) player_lives = 2;
            else player_lives = 1;

            APP_DBG("[GAME] Bat dau choi! Do kho: %d, Mang: %d\n", g_difficulty, player_lives);
            APP_DBG("[GAME] Ech xuat phat tai X:%d, Y:%d\n", frog_x, frog_y);
        } 
        else if (g_state == GAME_STATE_PLAYING) {
            // Nếu đang chơi, nút UP dùng để NHẢY TIẾN LÊN
            if (frog_y > 0) {
                frog_y -= 10; // Nhảy tiến lên 10 pixel
                APP_DBG("[GAME] Ech nhay TIEN! Toa do hien tai X:%d, Y:%d\n", frog_x, frog_y);
                
                // Giả lập check qua màn
                if (frog_y <= 10) {
                    APP_DBG("[GAME] CHUC MUNG! QUA MAN!\n");
                    g_state = GAME_STATE_MENU; // Quay về menu
                    frog_y = 60; // Reset vị trí
                }
            }
        }
        break;
    }

    default:
        break;
    }
}