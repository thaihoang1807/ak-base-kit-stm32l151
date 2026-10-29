#include "arcade.h"
#include "scr_idle.h"
#include "view_render.h"
#include "app_dbg.h"
#include "task_hello.h"

static void render_game();

view_dynamic_t dyn_view_idle = {
    { ITEM_TYPE_DYNAMIC },
    render_game
};

view_screen_t scr_idle = {
    { &dyn_view_idle, ITEM_NULL, ITEM_NULL },
    0
};

static void text(int x, int y, const char* s) {
    view_render.setCursor(x, y);
    view_render.print(s);
}

static const char* scene_name() {
    return g_scene == SCENE_ROAD ? "CITY ROAD" :
           g_scene == SCENE_RIVER ? "RIVER" : "FOREST";
}

void scr_idle_handle(ak_msg_t* msg) {
    if (msg->sig == SCREEN_ENTRY) {
        APP_DBG("[SCREEN] Frog landscapes ready\n");
    }
}

static void draw_frog() {
    if (respawn_ticks && respawn_ticks % 2 == 0) {
        return;
    }

    view_render.fillRect(
        frog_x - 1, frog_y - 2, 12, 11, BLACK);
    view_render.fillRect(
        frog_x, frog_y, 10, 8, WHITE);

    view_render.fillCircle(
        frog_x + 2, frog_y, 2, WHITE);
    view_render.fillCircle(
        frog_x + 7, frog_y, 2, WHITE);

    view_render.drawPixel(
        frog_x + 2, frog_y, BLACK);
    view_render.drawPixel(
        frog_x + 7, frog_y, BLACK);

    view_render.drawLine(
        frog_x - 1, frog_y + 3,
        frog_x, frog_y + 6, WHITE);

    view_render.drawLine(
        frog_x + 10, frog_y + 3,
        frog_x + 9, frog_y + 6, WHITE);
}

static void draw_car(int x, int y, int width) {
    view_render.fillRect(x, y, width, 6, WHITE);
    view_render.fillRect(x + 2, y + 1, 3, 3, BLACK);

    view_render.fillRect(x + 2, y - 1, 3, 2, WHITE);
    view_render.fillRect(
        x + width - 5, y - 1, 3, 2, WHITE);

    view_render.fillRect(x + 2, y + 5, 3, 2, WHITE);
    view_render.fillRect(
        x + width - 5, y + 5, 3, 2, WHITE);
}

static void draw_tree(int x, int y) {
    view_render.drawLine(
        x, y, x - 3, y + 4, WHITE);
    view_render.drawLine(
        x, y, x + 3, y + 4, WHITE);
    view_render.drawLine(
        x - 3, y + 4, x + 3, y + 4, WHITE);
    view_render.drawLine(
        x, y + 4, x, y + 6, WHITE);
}

static void draw_animal(int x, int y,
                        int width, bool right) {
    view_render.drawRect(
        x, y, width - 4, 5, WHITE);

    int head = right ? x + width - 4 : x;

    view_render.fillRect(head, y, 4, 4, WHITE);
    view_render.drawPixel(
        head + (right ? 2 : 1), y + 1, BLACK);

    view_render.drawLine(
        x + 2, y + 4, x + 1, y + 6, WHITE);

    view_render.drawLine(
        x + width - 6, y + 4,
        x + width - 5, y + 6, WHITE);
}

static void draw_landscape() {
    if (g_scene == SCENE_ROAD) {
        for (int x = 0; x < 128; x += 12) {
            view_render.drawLine(
                x, 25, x + 5, 25, WHITE);
            view_render.drawLine(
                x, 40, x + 5, 40, WHITE);
        }

        draw_car(car1_x, car1_y, 15);
        draw_car(car2_x, car2_y, 22);
        draw_car(car3_x, car3_y, 14);

        view_render.drawLine(
            0, 61, 127, 61, WHITE);

        for (int x = 0; x < 128; x += 10) {
            view_render.drawLine(
                x, 61, x, 63, WHITE);
        }
    } else if (g_scene == SCENE_RIVER) {
        // Vẽ mặt nước.
        for (int y = 18; y < 52; y += 6) {
            for (int x = (y % 12) ? 3 : 13;
                 x < 128; x += 20) {
                view_render.drawLine(
                    x, y, x + 3, y, WHITE);
            }
        }

        // Vẽ ba làn bè.
        for (int lane = 0; lane < 3; ++lane) {
            int y = 40 - lane * 12;
            int width = game_log_width();

            for (int k = -1; k <= 2; ++k) {
                int x = log_x[lane] + 68 * k;

                view_render.fillRect(
                    x, y, width, 12, BLACK);
                view_render.drawRect(
                    x, y, width, 12, WHITE);

                view_render.drawLine(
                    x + 3, y + 3,
                    x + width - 4, y + 3, WHITE);

                view_render.drawLine(
                    x + 4, y + 8,
                    x + width - 5, y + 8, WHITE);
            }
        }

        view_render.drawLine(
            0, 52, 127, 52, WHITE);

        for (int x = 8; x < 128; x += 20) {
            draw_tree(x, 56);
        }
    } else {
        // Cỏ và cây trong rừng.
        for (int y = 24; y <= 40; y += 8) {
            for (int x = 6; x < 128; x += 22) {
                view_render.drawLine(
                    x, y, x + 1, y - 2, WHITE);
                view_render.drawLine(
                    x + 1, y, x + 3, y - 1, WHITE);
            }
        }

        for (int x = 7; x < 128; x += 18) {
            draw_tree(x, 56);
        }

        draw_animal(car1_x, car1_y, 15, true);
        draw_animal(car2_x, car2_y, 22, false);
        draw_animal(car3_x, car3_y, 14, true);
    }

    // Bờ đích.
    view_render.drawLine(0, 15, 127, 15, WHITE);

    for (int x = 2; x < 128; x += 8) {
        view_render.drawPixel(x, 12, WHITE);
    }
}

static void draw_items() {
    if (coin_active) {
        view_render.fillCircle(
            coin_x + 3, coin_y + 3, 3, WHITE);

        view_render.drawLine(
            coin_x + 3, coin_y + 1,
            coin_x + 3, coin_y + 5, BLACK);
    }

    if (heart_active && player_lives < 3) {
        view_render.fillCircle(
            heart_x + 1, heart_y + 2, 1, WHITE);
        view_render.fillCircle(
            heart_x + 5, heart_y + 2, 1, WHITE);

        view_render.drawLine(
            heart_x, heart_y + 3,
            heart_x + 6, heart_y + 3, WHITE);

        view_render.drawLine(
            heart_x + 1, heart_y + 4,
            heart_x + 5, heart_y + 4, WHITE);

        view_render.drawLine(
            heart_x + 2, heart_y + 5,
            heart_x + 4, heart_y + 5, WHITE);

        view_render.drawPixel(
            heart_x + 3, heart_y + 6, WHITE);
    }
}

static void render_game() {
    if (arcade_render()) {
        return;
    }
    view_render.clear();
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setTextWrap(false);

    if (g_state == GAME_STATE_MENU) {
        view_render.drawRect(
            0, 0, 128, 64, WHITE);

        text(25, 5, "FROG JOURNEY");
        text(7, 18, "ROAD > RIVER > WOOD");

        text(7, 31, "DIFF:");

        view_render.print(
            g_difficulty == DIFF_EASY ? " EASY" :
            g_difficulty == DIFF_NORMAL
                ? " NORMAL" : " HARD"
        );

        text(7, 42, "TIME:");

        view_render.print(
            g_difficulty == DIFF_EASY ? 45 :
            g_difficulty == DIFF_NORMAL ? 35 : 25
        );

        view_render.print("s  HP:");

        view_render.print(
            g_difficulty == DIFF_EASY ? 3 :
            g_difficulty == DIFF_NORMAL ? 2 : 1
        );

        text(22, 54, "[UP] TO START");
    } else if (g_state == GAME_STATE_PLAYING) {
        draw_landscape();
        draw_items();
        draw_frog();

        // HUD: level-cảnh, điểm, thời gian và HP.
        view_render.fillRect(
            0, 0, 128, 9, WHITE);
        view_render.setTextColor(BLACK);

        text(1, 1, "");
        view_render.print(game_level);
        view_render.print('-');
        view_render.print(
            static_cast<int>(g_scene) + 1);

        text(25, 1, "S:");
        view_render.print(game_score);

        text(69, 1, "T:");
        view_render.print(game_time_seconds());

        text(104, 1, "H:");
        view_render.print(player_lives);

        view_render.setTextColor(WHITE);

        if (transition_ticks) {
            view_render.fillRect(
                8, 21, 112, 29, BLACK);
            view_render.drawRect(
                8, 21, 112, 29, WHITE);

            text(27, 25, "NEXT SCENE");
            text(33, 38, scene_name());
        }
    } else if (g_state == GAME_STATE_GAMEOVER) {
        view_render.drawRect(
            0, 0, 128, 64, WHITE);

        text(34, 8, "GAME OVER!");

        text(22, 23, "SCORE:");
        view_render.print(game_score);

        text(22, 35, "BEST :");
        view_render.print(best_score);

        text(16, 52, "Press UP to Menu");
    }
}
