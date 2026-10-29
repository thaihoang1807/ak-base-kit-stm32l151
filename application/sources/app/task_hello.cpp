#include "arcade.h"
#include "task_hello.h"
#include "app.h"
#include "task_list.h"
#include "timer.h"
#include "buzzer.h"
#include "port.h"

game_state_t g_state = GAME_STATE_MENU;
game_diff_t g_difficulty = DIFF_NORMAL;
game_scene_t g_scene = SCENE_ROAD;

int8_t frog_x = 59, frog_y = 52;

int16_t car1_x = 10, car1_y = 44;
int16_t car2_x = 100, car2_y = 28;
int16_t car3_x = 50, car3_y = 16;

int16_t log_x[3] = {20, 40, 0};

uint8_t player_lives = 2, game_level = 1;
uint8_t transition_ticks = 0, respawn_ticks = 0;

uint16_t game_score = 0, best_score = 0;
uint16_t time_ticks_left = 350;

int16_t coin_x = 78, coin_y = 34;
int16_t heart_x = 20, heart_y = 22;

bool coin_active = true;
bool heart_active = true;

static uint8_t sound_lock_ticks = 0;

// Một tick game = 100 ms.
uint8_t game_time_seconds() {
    return (time_ticks_left + 9) / 10;
}

static uint16_t time_budget() {
    return g_difficulty == DIFF_EASY ? 450 :
           g_difficulty == DIFF_NORMAL ? 350 : 250;
}

static void add_score(uint16_t value) {
    game_score = game_score <= 65535U - value
        ? game_score + value : 65535;

    if (game_score > best_score) {
        best_score = game_score;
    }
}

int16_t game_log_width() {
    return g_difficulty == DIFF_EASY ? 54 :
           g_difficulty == DIFF_NORMAL ? 48 : 44;
}

static void play_effect(buzzer_sound_t sound,
                        uint8_t lock_ticks = 0) {
    // Bảo vệ dữ liệu âm thanh đang được ngắt buzzer sử dụng.
    ENTRY_CRITICAL();
    BUZZER_Disable();
    BUZZER_PlaySound(sound);
    EXIT_CRITICAL();

    sound_lock_ticks = lock_ticks;
}

static void movement_click() {
    if (!sound_lock_ticks) {
        play_effect(BUZZER_SOUND_CLICK);
    }
}

static void redraw() {
    task_post_pure_msg(
        AC_TASK_DISPLAY_ID,
        AC_DISPLAY_RENDER_SCREEN
    );
}

static void reset_scene() {
    frog_x = 59;
    frog_y = 52;

    car1_x = 10;
    car2_x = 100;
    car3_x = 50;

    log_x[0] = 20;
    log_x[1] = 40;
    log_x[2] = 0;

    respawn_ticks = 0;
    time_ticks_left = time_budget();

    coin_active = true;
    heart_active = true;

    coin_x = 78;
    coin_y = 34;

    heart_x = 20;
    heart_y = 22;

    if (g_scene == SCENE_RIVER) {
        // Xu nằm trên một bè ở làn giữa.
        coin_x = log_x[1] + (game_log_width() - 6) / 2;
        coin_y = 30;

        // Tim nằm trên bờ xuất phát.
        heart_y = 54;
    }
}

static void lose_life() {
    if (player_lives) {
        --player_lives;
    }

    if (!player_lives) {
        g_state = GAME_STATE_GAMEOVER;

        play_effect(BUZZER_SOUND_LOWSCORE, 10);

        timer_remove_attr(
            AC_TASK_HELLO_ID,
            AC_TASK_HELLO_TICK
        );
    } else {
        frog_x = 59;
        frog_y = 52;

        respawn_ticks = 8;
        time_ticks_left = time_budget();

        play_effect(BUZZER_SOUND_BANG, 2);
    }
}

static bool overlap(int16_t x, int16_t y,
                    int16_t w, int16_t h) {
    return frog_x < x + w &&
           frog_x + 10 > x &&
           frog_y < y + h &&
           frog_y + 8 > y;
}

static int river_lane() {
    if (frog_y >= 52 || frog_y <= 10) {
        return -1;
    }

    return (51 - (frog_y + 4)) / 12;
}

static bool on_log(int lane) {
    const int width = game_log_width();

    for (int k = -1; k <= 2; ++k) {
        int left = log_x[lane] + 68 * k;

        if (frog_x >= left &&
            frog_x + 10 <= left + width) {
            return true;
        }
    }

    return false;
}

static void next_scene() {
    bool completed_round = g_scene == SCENE_FOREST;

    add_score(100);

    if (g_scene == SCENE_FOREST) {
        g_scene = SCENE_ROAD;

        if (game_level < 9) {
            ++game_level;
        }
    } else {
        g_scene = static_cast<game_scene_t>(g_scene + 1);
    }

    reset_scene();

    // Hiện tên cảnh mới trong 1,2 giây.
    transition_ticks = 12;

    play_effect(
        completed_round
            ? BUZZER_SOUND_HIGHSCORE
            : BUZZER_SOUND_WELCOME,
        12
    );
}

static void collect_items() {
    if (coin_active &&
        overlap(coin_x, coin_y, 6, 6)) {
        coin_active = false;

        add_score(25);
        play_effect(BUZZER_SOUND_USB_CONNECTED, 2);
    }

    if (heart_active &&
        player_lives < 3 &&
        overlap(heart_x, heart_y, 7, 7)) {
        heart_active = false;
        ++player_lives;

        play_effect(BUZZER_SOUND_WELCOME, 7);
    }
}

static void check_player() {
    if (frog_y <= 10) {
        next_scene();
        return;
    }

    if (g_scene == SCENE_RIVER) {
        int lane = river_lane();

        if (lane >= 0 && !on_log(lane)) {
            lose_life();
            return;
        }
    } else if (!respawn_ticks) {
        if (overlap(car1_x, car1_y, 15, 6) ||
            overlap(car2_x, car2_y, 22, 6) ||
            overlap(car3_x, car3_y, 14, 6)) {
            lose_life();
            return;
        }
    }

    // Chỉ nhặt vật phẩm sau khi kiểm tra nguy hiểm.
    collect_items();
}

static void move_hazard(int16_t& x,
                        int speed, int width) {
    x += speed;

    if (speed > 0 && x > 127) {
        x = -width;
    }

    if (speed < 0 && x < -width) {
        x = 128;
    }
}

static void tick() {
    if (sound_lock_ticks) {
        --sound_lock_ticks;
    }

    // Không trừ thời gian trong lúc chuyển cảnh.
    if (transition_ticks) {
        --transition_ticks;
        return;
    }

    if (time_ticks_left) {
        --time_ticks_left;
    }

    if (!time_ticks_left) {
        lose_life();
        return;
    }

    // Báo một lần mỗi giây trong 5 giây cuối.
    if (time_ticks_left <= 50 &&
        time_ticks_left % 10 == 0 &&
        !sound_lock_ticks) {
        play_effect(BUZZER_SOUND_USB_DISCONNECTED, 2);
    }

    if (respawn_ticks) {
        --respawn_ticks;
    }

    int extra = (game_level - 1) / 2;
    int mult = g_difficulty == DIFF_HARD ? 2 : 1;

    if (g_scene == SCENE_RIVER) {
        int lane = river_lane();
        bool riding = lane >= 0 && on_log(lane);

        int speed[3] = {
            1 + extra,
            -(1 + extra),
            1 + extra
        };

        for (int i = 0; i < 3; ++i) {
            speed[i] *= mult;
            log_x[i] =
                (log_x[i] + speed[i] + 68) % 68;
        }

        // Đồng xu trôi cùng bè ở làn giữa.
        if (coin_active) {
            coin_x += speed[1];

            if (coin_x < -6) {
                coin_x += 136;
            }

            if (coin_x > 129) {
                coin_x -= 136;
            }
        }

        if (riding) {
            int x = frog_x + speed[lane];

            if (x < 2 || x > 116) {
                lose_life();
                return;
            }

            frog_x = static_cast<int8_t>(x);
        }
    } else {
        move_hazard(
            car1_x, (2 + extra) * mult, 15);
        move_hazard(
            car2_x, -(3 + extra) * mult, 22);
        move_hazard(
            car3_x, (4 + extra) * mult, 14);
    }

    check_player();
}

void task_hello(ak_msg_t* msg) {
    if (arcade_handle(msg)) {
        return;
    }
    switch (msg->sig) {
    case AC_TASK_HELLO_PRINT:
        if (g_state == GAME_STATE_MENU) {
            g_state = GAME_STATE_PLAYING;
            g_scene = SCENE_ROAD;

            game_score = 0;
            game_level = 1;

            player_lives =
                g_difficulty == DIFF_EASY ? 3 :
                g_difficulty == DIFF_NORMAL ? 2 : 1;

            transition_ticks = 0;

            reset_scene();
            play_effect(BUZZER_SOUND_WELCOME, 7);

            task_post_pure_msg(
                AC_TASK_DISPLAY_ID,
                AC_DISPLAY_SHOW_IDLE
            );

            timer_set(
                AC_TASK_HELLO_ID,
                AC_TASK_HELLO_TICK,
                100,
                TIMER_PERIODIC
            );
        } else if (g_state == GAME_STATE_PLAYING &&
                   !transition_ticks) {
            movement_click();

            frog_y -= 6;
            check_player();
        } else if (g_state == GAME_STATE_GAMEOVER) {
            g_state = GAME_STATE_MENU;
            play_effect(BUZZER_SOUND_CLICK);
        }

        break;

    case AC_TASK_HELLO_LEFT:
    case AC_TASK_HELLO_RIGHT: {
        int direction =
            msg->sig == AC_TASK_HELLO_LEFT ? -1 : 1;

        if (g_state == GAME_STATE_MENU) {
            int d =
                static_cast<int>(g_difficulty) + direction;

            if (d < DIFF_EASY) {
                d = DIFF_EASY;
            }

            if (d > DIFF_HARD) {
                d = DIFF_HARD;
            }

            if (d != static_cast<int>(g_difficulty)) {
                play_effect(BUZZER_SOUND_CLICK);
            }

            g_difficulty = static_cast<game_diff_t>(d);
        } else if (g_state == GAME_STATE_PLAYING &&
                   !transition_ticks) {
            int x = frog_x + direction * 6;

            if (x < 2) {
                x = 2;
            }

            if (x > 116) {
                x = 116;
            }

            if (x != frog_x) {
                movement_click();
            }

            frog_x = static_cast<int8_t>(x);
            check_player();
        }

        break;
    }

    case AC_TASK_HELLO_TICK:
        if (g_state == GAME_STATE_PLAYING) {
            tick();
        }

        break;

    default:
        return;
    }

    redraw();
}
