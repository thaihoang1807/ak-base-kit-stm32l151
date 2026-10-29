#include "scr_startup.h"
#include "view_render.h"
#include "task_list.h"
#include "timer.h"
#include "screens.h"

static void view_scr_startup();

view_dynamic_t dyn_view_startup = {
    { ITEM_TYPE_DYNAMIC },
    view_scr_startup
};

view_screen_t scr_startup = {
    { &dyn_view_startup, ITEM_NULL, ITEM_NULL },
    0
};

void scr_startup_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case AC_DISPLAY_INITIAL:
        view_render.initialize();
        view_render_display_on();

        timer_set(
            AC_TASK_DISPLAY_ID,
            AC_DISPLAY_SHOW_IDLE,
            AC_DISPLAY_STARTUP_INTERVAL,
            TIMER_ONE_SHOT
        );
        break;

    case AC_DISPLAY_SHOW_IDLE:
        SCREEN_TRAN(scr_idle_handle, &scr_idle);
        break;

    default:
        break;
    }
}

static void view_scr_startup() {
    view_render.clear();
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setTextWrap(false);

    // Ten chung cho bo game.
    view_render.setCursor(22, 5);
    view_render.print("AK MINI ARCADE");

    // Hinh tay cam.
    view_render.drawRoundRect(
        33, 21, 62, 26, 6, WHITE
    );

    // Cum phim dieu huong.
    view_render.fillRect(42, 29, 4, 12, WHITE);
    view_render.fillRect(38, 33, 12, 4, WHITE);

    // Hai nut bam.
    view_render.fillCircle(79, 36, 2, WHITE);
    view_render.fillCircle(86, 29, 2, WHITE);

    // Nut giua tay cam.
    view_render.drawLine(59, 36, 63, 36, WHITE);
    view_render.drawLine(66, 33, 70, 33, WHITE);

    view_render.setCursor(31, 54);
    view_render.print("Starting...");
}
