#include "scr_startup.h"
#include "view_render.h"
#include "app_dbg.h"

// Gọi biến độ khó từ task_hello sang để vẽ lên màn hình chào
typedef enum {
    DIFF_EASY,
    DIFF_NORMAL,
    DIFF_HARD
} game_diff_t;
extern game_diff_t g_difficulty;

void scr_startup_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY:
		APP_DBG("[SCREEN] Da vao man hinh Startup!\n");
		break;
	default:
		break;
	}
}

void scr_startup_render() {
	view_render.draw_string(20, 20, "FROG CROSSING");
    
    // Vẽ Menu chọn độ khó
    view_render.draw_string(10, 40, "Select Difficulty:");
    if (g_difficulty == DIFF_EASY) {
        view_render.draw_string(35, 55, "<< EASY >>");
    } else if (g_difficulty == DIFF_NORMAL) {
        view_render.draw_string(30, 55, "<< NORMAL >>");
    } else if (g_difficulty == DIFF_HARD) {
        view_render.draw_string(35, 55, "<< HARD >>");
    }
}
