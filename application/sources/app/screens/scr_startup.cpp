#include "scr_startup.h"
#include "view_render.h"
#include "task_list.h"
#include "timer.h"
#include "screens.h"

static void view_scr_startup(); 
view_dynamic_t dyn_view_startup = { { .item_type = ITEM_TYPE_DYNAMIC, }, view_scr_startup };
view_screen_t scr_startup = { &dyn_view_startup, ITEM_NULL, ITEM_NULL, .focus_item = 0 };

void scr_startup_handle(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_DISPLAY_INITIAL:
        // app_start_timer() sends this event after SCREEN_CTOR().
        // Initialize the OLED before screen_manager renders its first frame.
        view_render.initialize();
        view_render_display_on();
        timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_IDLE,
                  AC_DISPLAY_STARTUP_INTERVAL, TIMER_ONE_SHOT);
		break;
        
    // Show the game menu after the startup screen.
    case AC_DISPLAY_SHOW_IDLE:
        SCREEN_TRAN(scr_idle_handle, &scr_idle);
        break;

	default: break;
	}
}

void view_scr_startup() {
    view_render.setTextColor(1);
    view_render.setTextSize(1);
	view_render.setCursor(20, 25); 
    view_render.print("FROG CROSSING");
    view_render.setCursor(35, 45); 
    view_render.print("Loading...");
}
