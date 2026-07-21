#include "scr_idle.h"
#include "view_render.h"
#include "app_dbg.h"
#include "app.h"
#include "task_list.h"
#include "task_hello.h"
#include <stdio.h> 

// --- GỌI CÁC BIẾN TỪ GAME SANG ---
extern int8_t frog_x; extern int8_t frog_y;
extern int16_t car1_x;  extern int16_t car1_y;
extern int16_t car2_x;  extern int16_t car2_y;
extern int16_t car3_x;  extern int16_t car3_y;
extern uint8_t player_lives; extern uint16_t game_score;
extern game_state_t g_state; extern game_diff_t g_difficulty;

// =========================================================================
// CẤU TRÚC GỐC CỦA HỆ ĐIỀU HÀNH (ĐƯỢC LẤY TỪ GITHUB CỦA BẠN - CẤM ĐỤNG VÀO)
// =========================================================================
static void view_scr_idle(); // Khai báo hàm vẽ

view_dynamic_t dyn_view_idle = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_idle // Hệ điều hành sẽ tự động gọi hàm này để vẽ lên LCD
};

view_screen_t scr_idle = {
	&dyn_view_idle,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0,
};
// =========================================================================


// --- HÀM XỬ LÝ SỰ KIỆN MÀN HÌNH ---
void scr_idle_handle(ak_msg_t *msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY:
		APP_DBG("[SCREEN] Man hinh Game HD san sang!\n");
		break;
        
    // Bắt tín hiệu từ task_hello gửi sang để hệ thống biết mà vẽ lại
    case AC_DISPLAY_SHOW_IDLE_BALL_MOVING_UPDATE:
    case AC_DISPLAY_RENDER_SCREEN:
        break;

	default:
		break;
	}
}

// --- HÀM VẼ GIAO DIỆN GAME (THAY THẾ CHO BÓNG LĂN CŨ) ---
void view_scr_idle() {
    view_render.setTextColor(1);
    view_render.setTextSize(1);

    if (g_state == GAME_STATE_MENU) {
        /* VẼ MÀN HÌNH MENU CHỌN ĐỘ KHÓ */
        view_render.drawRect(0, 0, 128, 64, 1);
        view_render.drawRect(2, 2, 124, 60, 1); 
        view_render.setCursor(15, 15); view_render.print("- FROG CROSSING -");
        view_render.drawLine(15, 25, 113, 25, 1); 
        view_render.setCursor(10, 36); view_render.print("Select Difficulty:");
        if (g_difficulty == DIFF_EASY) { view_render.setCursor(35, 48); view_render.print("<< EASY >>"); } 
        else if (g_difficulty == DIFF_NORMAL) { view_render.setCursor(30, 48); view_render.print("<< NORMAL >>"); } 
        else if (g_difficulty == DIFF_HARD) { view_render.setCursor(35, 48); view_render.print("<< HARD >>"); }
    }
    else if (g_state == GAME_STATE_PLAYING) {
        /* 1. KHUNG GIAO DIỆN CHUNG */
        view_render.drawRect(0, 0, 128, 64, 1);
        view_render.drawLine(0, 10, 128, 10, 1);
        char score_str[15]; char hp_str[10];
        sprintf(score_str, "SC:%d", game_score); sprintf(hp_str, "HP:%d", player_lives);
        view_render.setCursor(2, 1); view_render.print(score_str);
        view_render.setCursor(90, 1); view_render.print(hp_str);

        /* 2. VẼ ĐƯỜNG XÁ */
        view_render.setCursor(40, 12); view_render.print("--- GOAL ---");
        for (int i = 0; i < 128; i += 12) {
            view_render.drawLine(i, 25, i + 6, 25, 1);
            view_render.drawLine(i, 40, i + 6, 40, 1);
        }

        /* 3. VẼ XE Ô TÔ */
        view_render.fillRect(car1_x, car1_y, 15, 6, 1);
        view_render.fillRect(car1_x + 2, car1_y - 1, 3, 2, 1);
        view_render.fillRect(car1_x + 10, car1_y - 1, 3, 2, 1);
        view_render.fillRect(car1_x + 2, car1_y + 5, 3, 2, 1);
        view_render.fillRect(car1_x + 10, car1_y + 5, 3, 2, 1);

        view_render.fillRect(car2_x, car2_y, 22, 6, 1);
        view_render.drawRect(car2_x + 1, car2_y + 1, 4, 4, 0); 
        view_render.fillRect(car2_x + 3, car2_y - 1, 4, 2, 1);
        view_render.fillRect(car2_x + 15, car2_y - 1, 4, 2, 1);
        view_render.fillRect(car2_x + 3, car2_y + 5, 4, 2, 1);
        view_render.fillRect(car2_x + 15, car2_y + 5, 4, 2, 1);

        view_render.drawRect(car3_x, car3_y, 12, 6, 1);
        view_render.fillRect(car3_x + 2, car3_y + 2, 8, 2, 1);

        /* 4. VẼ CON ẾCH */
        view_render.fillRect(frog_x, frog_y, 10, 8, 1); 
        view_render.fillCircle(frog_x + 2, frog_y - 1, 2, 1);
        view_render.drawCircle(frog_x + 2, frog_y - 1, 1, 0);
        view_render.fillCircle(frog_x + 8, frog_y - 1, 2, 1);
        view_render.drawCircle(frog_x + 8, frog_y - 1, 1, 0);
        view_render.drawLine(frog_x - 2, frog_y + 2, frog_x, frog_y + 4, 1);
        view_render.drawLine(frog_x - 2, frog_y + 6, frog_x, frog_y + 4, 1);
        view_render.drawLine(frog_x + 11, frog_y + 2, frog_x + 9, frog_y + 4, 1);
        view_render.drawLine(frog_x + 11, frog_y + 6, frog_x + 9, frog_y + 4, 1);
    }
    else if (g_state == GAME_STATE_GAMEOVER) {
        /* VẼ MÀN HÌNH GAME OVER */
        view_render.drawRect(0, 0, 128, 64, 1);
        view_render.drawRect(2, 2, 124, 60, 1);
        view_render.setCursor(30, 20); view_render.print("GAME OVER!");
        view_render.setCursor(15, 35); view_render.print("Try again!");
        view_render.setCursor(15, 50); view_render.print("Press UP to Menu");
    }
}
