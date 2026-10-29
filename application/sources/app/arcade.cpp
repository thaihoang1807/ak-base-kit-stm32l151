#include "arcade.h"
#include "task_hello.h"
#include "task_list.h"
#include "timer.h"
#include "view_render.h"
#include "buzzer.h"
#include "port.h"
#include <string.h>

namespace {

enum Game {
    LAUNCHER = -1,
    FROG,
    SNAKE,
    PONG,
    DODGE,
    FLAPPY,
    GAME_COUNT
};

Game active = LAUNCHER;
uint8_t selected = 0;

bool ready = false;
bool paused = false;
bool ended = false;
bool won = false;

uint16_t score = 0;
uint16_t frame = 0;
uint16_t flappy_best = 0;
uint32_t random_state = 17431;

struct SnakeData {
    uint8_t body[160];
    uint8_t length, direction, food, divider;
    bool turned;
};

struct PongData {
    int16_t paddle, x, y, dx, dy;
    uint32_t bricks;
    uint8_t lives, serve;
};

struct DodgeData {
    int16_t y[3];
    uint8_t lane[3];
    uint8_t player, lives, shield, scroll;
};

struct FlappyData {
    // Vi tri va van toc theo don vi 1/16 pixel.
    int16_t y, velocity;
    int16_t pipe_x[2];
    uint8_t gap_top[2];
    bool passed[2];
};

union GameData {
    SnakeData snake;
    PongData pong;
    DodgeData dodge;
    FlappyData flappy;
} data;

static_assert(
    sizeof(GameData) <= 192,
    "Mini game RAM too large"
);

const char* game_name(int game) {
    static const char* const names[] = {
        "FROG JOURNEY",
        "SNAKE",
        "BRICK PONG",
        "ROAD DODGE",
        "FLAPPY BIRD"
    };

    return names[game];
}

int random_range(int limit) {
    random_state =
        random_state * 1664525UL + 1013904223UL;

    return (random_state >> 16) % limit;
}

void sound(buzzer_sound_t effect) {
    ENTRY_CRITICAL();
    BUZZER_Disable();
    BUZZER_PlaySound(effect);
    EXIT_CRITICAL();
}

void redraw() {
    task_post_pure_msg(
        AC_TASK_DISPLAY_ID,
        AC_DISPLAY_RENDER_SCREEN
    );
}

void stop_timer() {
    timer_remove_attr(
        AC_TASK_HELLO_ID,
        AC_TASK_HELLO_TICK
    );
}

void finish(bool victory = false) {
    ended = true;
    won = victory;
    paused = false;

    if (active == FLAPPY && score > flappy_best) {
        flappy_best = score;
    }

    stop_timer();

    sound(
        victory
            ? BUZZER_SOUND_HIGHSCORE
            : BUZZER_SOUND_LOWSCORE
    );
}

bool occupied(int cell, int count) {
    for (int i = 0; i < count; ++i) {
        if (data.snake.body[i] == cell) {
            return true;
        }
    }

    return false;
}

void place_food() {
    int free_cells = 160 - data.snake.length;

    if (!free_cells) {
        finish(true);
        return;
    }

    int target = random_range(free_cells);

    for (int cell = 0; cell < 160; ++cell) {
        if (!occupied(cell, data.snake.length) &&
            target-- == 0) {
            data.snake.food = cell;
            return;
        }
    }
}

void serve_ball() {
    PongData& p = data.pong;

    p.x = p.paddle + 11;
    p.y = 53;
    p.dx = random_range(2) ? 2 : -2;
    p.dy = -2;
    p.serve = 20;
}

void start_mini() {
    stop_timer();
    memset(&data, 0, sizeof(data));

    score = frame = 0;
    ready = true;
    paused = ended = won = false;

    if (active == SNAKE) {
        SnakeData& s = data.snake;
        s.length = 4;

        for (int i = 0; i < 4; ++i) {
            s.body[i] = 88 - i;
        }

        place_food();
    } else if (active == PONG) {
        data.pong.paddle = 52;
        data.pong.lives = 3;
        data.pong.bricks = 0xFFFFFFUL;
        serve_ball();
    } else if (active == FLAPPY) {
        FlappyData& f = data.flappy;

        f.y = 28 * 16;

        for (int i = 0; i < 2; ++i) {
            f.pipe_x[i] = 140 + i * 80;
            f.gap_top[i] = 17 + random_range(15);
        }
    } else {
        DodgeData& d = data.dodge;
        d.player = 1;
        d.lives = 3;

        for (int i = 0; i < 3; ++i) {
            d.lane[i] = random_range(3);
            d.y[i] = -8 - i * 36;
        }
    }

    sound(BUZZER_SOUND_CLICK);

    timer_set(
        AC_TASK_HELLO_ID,
        AC_TASK_HELLO_TICK,
        50,
        TIMER_PERIODIC
    );
}

void snake_tick() {
    SnakeData& s = data.snake;

    int period =
        score < 50 ? 6 : score < 120 ? 4 : 3;

    if (++s.divider < period) {
        return;
    }

    s.divider = 0;
    s.turned = false;

    static const int8_t dx[] = {1, 0, -1, 0};
    static const int8_t dy[] = {0, 1, 0, -1};

    int x = s.body[0] % 20 + dx[s.direction];
    int y = s.body[0] / 20 + dy[s.direction];

    if (x < 0 || x >= 20 || y < 0 || y >= 8) {
        finish();
        return;
    }

    int next = y * 20 + x;
    bool eating = next == s.food;

    if (occupied(
            next,
            s.length - (eating ? 0 : 1))) {
        finish();
        return;
    }

    if (eating) {
        ++s.length;
    }

    for (int i = s.length - 1; i > 0; --i) {
        s.body[i] = s.body[i - 1];
    }

    s.body[0] = next;

    if (eating) {
        score += 10;
        sound(BUZZER_SOUND_USB_CONNECTED);
        place_food();
    }
}

void pong_tick() {
    PongData& p = data.pong;

    if (p.serve) {
        --p.serve;
        p.x = p.paddle + 11;
        return;
    }

    int old_x = p.x;
    int old_y = p.y;

    p.x += p.dx;
    p.y += p.dy;

    if (p.x <= 2) {
        p.x = 2;
        if (p.dx < 0) p.dx = -p.dx;
    }

    if (p.x >= 122) {
        p.x = 122;
        if (p.dx > 0) p.dx = -p.dx;
    }

    if (p.y <= 12) {
        p.y = 12;
        if (p.dy < 0) p.dy = -p.dy;
    }

    for (int i = 0; i < 24; ++i) {
        if (!(p.bricks & (1UL << i))) {
            continue;
        }

        int x = 5 + (i % 8) * 15;
        int y = 14 + (i / 8) * 7;

        if (p.x + 3 > x &&
            p.x < x + 13 &&
            p.y + 3 > y &&
            p.y < y + 5) {
            p.bricks &= ~(1UL << i);
            score += 10;

            if (old_y + 3 <= y) {
                p.y = y - 3;
                p.dy = -p.dy;
            } else if (old_y >= y + 5) {
                p.y = y + 5;
                p.dy = -p.dy;
            } else {
                p.x = old_x < x ? x - 3 : x + 13;
                p.dx = -p.dx;
            }

            sound(BUZZER_SOUND_CLICK);

            if (!p.bricks) {
                finish(true);
                return;
            }

            break;
        }
    }

    if (p.dy > 0 &&
        p.y + 3 >= 58 &&
        p.y < 61 &&
        p.x + 3 > p.paddle &&
        p.x < p.paddle + 24) {
        p.y = 55;
        p.dy = -2;

        int offset =
            p.x + 1 - (p.paddle + 12);

        p.dx =
            offset < -5 ? -2 :
            offset < 0 ? -1 :
            offset > 5 ? 2 : 1;

        sound(BUZZER_SOUND_CLICK);
    }

    if (p.y > 63) {
        if (--p.lives == 0) {
            finish();
        } else {
            sound(BUZZER_SOUND_BANG);
            serve_ball();
        }
    }
}

void dodge_tick() {
    DodgeData& d = data.dodge;

    if (d.shield) {
        --d.shield;
    }

    int speed = 1 + score / 50;

    if (speed > 3) {
        speed = 3;
    }

    d.scroll = (d.scroll + speed) % 12;

    for (int i = 0; i < 3; ++i) {
        d.y[i] += speed;

        if (d.y[i] > 64) {
            d.y[i] -= 108;
            d.lane[i] = random_range(3);

            if (score <= 65530) {
                score += 5;
            }
        }

        if (!d.shield &&
            d.lane[i] == d.player &&
            d.y[i] < 61 &&
            d.y[i] + 14 > 49) {
            if (--d.lives == 0) {
                finish();
                return;
            }

            d.shield = 24;
            d.y[i] -= 108;
            d.lane[i] = random_range(3);

            sound(BUZZER_SOUND_BANG);
        }
    }
}

void flap() {
    data.flappy.velocity = -42;
    sound(BUZZER_SOUND_CLICK);
}

void flappy_tick() {
    FlappyData& f = data.flappy;

    // Trong luc.
    f.velocity += 4;

    if (f.velocity > 48) {
        f.velocity = 48;
    }

    f.y += f.velocity;

    int y = f.y / 16;

    // Va cham tran hoac mat dat.
    if (f.y < 12 * 16 ||
        f.y + 7 * 16 > 60 * 16) {
        finish();
        return;
    }

    int speed = score < 15 ? 2 : 3;

    for (int i = 0; i < 2; ++i) {
        f.pipe_x[i] -= speed;

        if (f.pipe_x[i] < -14) {
            f.pipe_x[i] =
                f.pipe_x[1 - i] + 80;

            f.gap_top[i] =
                17 + random_range(15);

            f.passed[i] = false;
        }

        int x = f.pipe_x[i];
        int gap = f.gap_top[i];

        // Chim rong 11, cao 7; khe ong cao 25.
        if (26 < x + 14 &&
            37 > x &&
            (y < gap || y + 7 > gap + 25)) {
            finish();
            return;
        }

        if (!f.passed[i] && x + 14 <= 26) {
            f.passed[i] = true;

            if (score < 65535) {
                ++score;
            }

            if (score > flappy_best) {
                flappy_best = score;
            }

            sound(BUZZER_SOUND_USB_CONNECTED);
        }
    }
}

void text(int x, int y, const char* s) {
    view_render.setCursor(x, y);
    view_render.print(s);
}

void car(int x, int y, bool player) {
    view_render.fillRect(x + 2, y, 10, 14, WHITE);
    view_render.fillRect(x + 3, y + 3, 8, 3, BLACK);
    view_render.fillRect(x + 3, y + 9, 8, 2, BLACK);

    view_render.fillRect(x, y + 2, 2, 4, WHITE);
    view_render.fillRect(x + 12, y + 2, 2, 4, WHITE);
    view_render.fillRect(x, y + 9, 2, 4, WHITE);
    view_render.fillRect(x + 12, y + 9, 2, 4, WHITE);

    if (player) {
        view_render.drawLine(
            x + 6, y + 6, x + 6, y + 8, BLACK);
    }
}

void draw_snake() {
    SnakeData& s = data.snake;

    view_render.drawRect(3, 13, 122, 50, WHITE);

    for (int i = s.length - 1; i >= 0; --i) {
        int x = 4 + s.body[i] % 20 * 6;
        int y = 14 + s.body[i] / 20 * 6;

        view_render.fillRect(x, y, 5, 5, WHITE);

        if (i) {
            view_render.drawPixel(
                x + 2, y + 2, BLACK);
        } else if (s.direction % 2 == 0) {
            int eye =
                s.direction == 0 ? x + 3 : x + 1;

            view_render.drawPixel(eye, y + 1, BLACK);
            view_render.drawPixel(eye, y + 3, BLACK);
        } else {
            int eye =
                s.direction == 1 ? y + 3 : y + 1;

            view_render.drawPixel(x + 1, eye, BLACK);
            view_render.drawPixel(x + 3, eye, BLACK);
        }
    }

    int x = 6 + s.food % 20 * 6;
    int y = 16 + s.food / 20 * 6;

    view_render.fillCircle(
        x, y, (frame / 5) % 2 ? 2 : 1, WHITE);

    view_render.drawPixel(x + 1, y - 2, WHITE);
}

void draw_pong() {
    PongData& p = data.pong;

    view_render.drawLine(1, 11, 1, 63, WHITE);
    view_render.drawLine(126, 11, 126, 63, WHITE);

    for (int i = 0; i < 24; ++i) {
        if (!(p.bricks & (1UL << i))) {
            continue;
        }

        int x = 5 + i % 8 * 15;
        int y = 14 + i / 8 * 7;

        if (i / 8 == 1) {
            view_render.drawRect(x, y, 13, 5, WHITE);
        } else {
            view_render.fillRect(x, y, 13, 5, WHITE);
        }
    }

    view_render.fillRect(
        p.paddle, 58, 24, 3, WHITE);

    view_render.drawPixel(
        p.paddle + 2, 59, BLACK);

    view_render.drawPixel(
        p.paddle + 21, 59, BLACK);

    view_render.fillRect(p.x, p.y, 3, 3, WHITE);

    for (int i = 0; i < p.lives; ++i) {
        view_render.drawPixel(
            6 + i * 4, 62, WHITE);
    }

    if (p.serve) {
        text(46, 42, "READY");
    }
}

void draw_dodge() {
    DodgeData& d = data.dodge;

    view_render.drawLine(3, 11, 3, 63, WHITE);
    view_render.drawLine(124, 11, 124, 63, WHITE);

    for (int y = 11 + d.scroll;
         y < 64; y += 12) {
        view_render.drawLine(
            44, y, 44, y + 5, WHITE);

        view_render.drawLine(
            84, y, 84, y + 5, WHITE);
    }

    for (int i = 0; i < 3; ++i) {
        if (d.y[i] >= 11) {
            car(
                17 + 40 * d.lane[i],
                d.y[i],
                false
            );
        }
    }

    if (!d.shield || (d.shield / 2) % 2) {
        car(17 + 40 * d.player, 49, true);
    }
}

void draw_flappy() {
    FlappyData& f = data.flappy;

    // May troi cham hon ong.
    for (int i = 0; i < 2; ++i) {
        int x = 130 - static_cast<int>(
            (frame / 3 + i * 67) % 150
        );

        view_render.drawCircle(
            x, 19 + i * 8, 3, WHITE);

        view_render.drawCircle(
            x + 5, 18 + i * 8, 4, WHITE);

        view_render.drawLine(
            x - 3, 22 + i * 8,
            x + 9, 22 + i * 8, WHITE);
    }

    for (int i = 0; i < 2; ++i) {
        int x = f.pipe_x[i];
        int top = f.gap_top[i];
        int bottom = top + 25;

        // Ong tren.
        view_render.fillRect(
            x + 2, 11, 10, top - 11, BLACK);

        view_render.drawRect(
            x + 2, 11, 10, top - 11, WHITE);

        view_render.fillRect(
            x, top - 3, 14, 3, WHITE);

        // Ong duoi.
        view_render.fillRect(
            x + 2, bottom, 10, 60 - bottom, BLACK);

        view_render.drawRect(
            x + 2, bottom, 10, 60 - bottom, WHITE);

        view_render.fillRect(
            x, bottom, 14, 3, WHITE);
    }

    // Chim.
    int y = f.y / 16;

    view_render.fillRect(26, y, 11, 7, BLACK);
    view_render.fillCircle(30, y + 3, 3, WHITE);
    view_render.fillRect(34, y + 2, 3, 2, WHITE);
    view_render.drawPixel(31, y + 1, BLACK);

    view_render.drawLine(
        27,
        y + ((frame % 4 < 2) ? 4 : 5),
        29, y + 3,
        BLACK
    );

    // Mat dat cuon.
    view_render.drawLine(0, 60, 127, 60, WHITE);

    for (int x = -static_cast<int>(frame % 6);
         x < 128; x += 6) {
        view_render.drawLine(
            x, 62, x + 2, 62, WHITE);
    }
}

} // namespace

bool arcade_handle(ak_msg_t* msg) {
    if (msg->sig == ARCADE_EXIT_SIG) {
        stop_timer();

        active = LAUNCHER;
        g_state = GAME_STATE_MENU;

        transition_ticks = respawn_ticks = 0;
        ready = paused = ended = won = false;

        sound(BUZZER_SOUND_CLICK);

        task_post_pure_msg(
            AC_TASK_DISPLAY_ID,
            AC_DISPLAY_SHOW_IDLE
        );

        redraw();
        return true;
    }

    if (active == FROG) {
        return false;
    }

    if (active == LAUNCHER) {
        if (msg->sig == AC_TASK_HELLO_LEFT) {
            selected =
                (selected + GAME_COUNT - 1) %
                GAME_COUNT;

            sound(BUZZER_SOUND_CLICK);
        } else if (msg->sig == AC_TASK_HELLO_RIGHT) {
            selected =
                (selected + 1) % GAME_COUNT;

            sound(BUZZER_SOUND_CLICK);
        } else if (msg->sig == AC_TASK_HELLO_PRINT) {
            stop_timer();
            active = static_cast<Game>(selected);

            if (active == FROG) {
                g_state = GAME_STATE_MENU;
            } else {
                start_mini();
            }

            task_post_pure_msg(
                AC_TASK_DISPLAY_ID,
                AC_DISPLAY_SHOW_IDLE
            );
        }

        redraw();
        return true;
    }

    if (msg->sig == AC_TASK_HELLO_PRINT) {
        if (ended) {
            start_mini();
        } else if (ready) {
            ready = false;

            if (active == FLAPPY) {
                flap();
            } else {
                sound(BUZZER_SOUND_CLICK);
            }
        } else if (active == FLAPPY) {
            if (paused) {
                paused = false;
            }

            flap();
        } else {
            paused = !paused;
            sound(BUZZER_SOUND_CLICK);
        }
    } else if (
        active == FLAPPY &&
        msg->sig == AC_TASK_HELLO_LEFT &&
        !ended && !ready
    ) {
        paused = !paused;
        sound(BUZZER_SOUND_CLICK);
    } else if (!ended && !paused && !ready) {
        if (msg->sig == AC_TASK_HELLO_TICK) {
            ++frame;

            if (active == SNAKE) {
                snake_tick();
            } else if (active == PONG) {
                pong_tick();
            } else if (active == FLAPPY) {
                flappy_tick();
            } else {
                dodge_tick();
            }
        } else if (
            msg->sig == AC_TASK_HELLO_LEFT ||
            msg->sig == AC_TASK_HELLO_RIGHT
        ) {
            int turn =
                msg->sig == AC_TASK_HELLO_LEFT
                    ? -1 : 1;

            if (active == SNAKE) {
                if (!data.snake.turned) {
                    data.snake.direction =
                        (data.snake.direction +
                         turn + 4) % 4;

                    data.snake.turned = true;
                }
            } else if (active == PONG) {
                int x =
                    data.pong.paddle + turn * 8;

                data.pong.paddle =
                    x < 2 ? 2 : x > 102 ? 102 : x;
            } else if (active == DODGE) {
                int lane =
                    data.dodge.player + turn;

                data.dodge.player =
                    lane < 0 ? 0 :
                    lane > 2 ? 2 : lane;
            }
        }
    }

    redraw();
    return true;
}

bool arcade_render() {
    if (active == FROG) {
        return false;
    }

    view_render.clear();
    view_render.setTextSize(1);
    view_render.setTextWrap(false);
    view_render.setTextColor(WHITE);

    if (active == LAUNCHER) {
        text(22, 1, "AK MINI ARCADE");

        // Hien 4 dong, cuon khi chon game thu 5.
        int first =
            selected < 4 ? 0 : selected - 3;

        for (int row = 0; row < 4; ++row) {
            int i = first + row;
            int y = 13 + row * 10;

            if (i == selected) {
                view_render.fillRect(
                    4, y - 1, 120, 9, WHITE);

                view_render.setTextColor(BLACK);
            }

            text(8, y, i == selected ? ">" : " ");
            text(22, y, game_name(i));

            view_render.setTextColor(WHITE);
        }

        text(7, 56, "DOWN/MODE UP:GO");
        return true;
    }

    if (ready) {
        text(20, 3, game_name(active));

        if (active == SNAKE) {
            text(10, 18, "DOWN: TURN LEFT");
            text(10, 29, "MODE: TURN RIGHT");
            text(10, 40, "EAT FRUIT +10");
        } else if (active == PONG) {
            text(10, 18, "DOWN/MODE: MOVE");
            text(10, 29, "BREAK 24 BRICKS");
            text(10, 40, "3 LIVES / +10");
        } else if (active == FLAPPY) {
            text(10, 18, "UP: FLAP WINGS");
            text(10, 29, "DOWN: PAUSE");
            text(10, 40, "PASS PIPES +1");
        } else {
            text(10, 18, "DOWN/MODE: LANE");
            text(10, 29, "AVOID THE CARS");
            text(10, 40, "3 LIVES / +5");
        }

        text(37, 54, "UP: START");
        return true;
    }

    if (ended) {
        text(
            34, 5,
            won ? "YOU WIN!" : "GAME OVER"
        );

        text(22, 19, "SCORE:");
        view_render.print(score);

        if (active == FLAPPY) {
            text(22, 29, "BEST :");
            view_render.print(flappy_best);
        }

        text(28, 41, "UP: RETRY");
        text(16, 53, "HOLD MODE: MENU");

        return true;
    }

    if (active == SNAKE) {
        draw_snake();
    } else if (active == PONG) {
        draw_pong();
    } else if (active == FLAPPY) {
        draw_flappy();
    } else {
        draw_dodge();
    }

    view_render.fillRect(0, 0, 128, 10, BLACK);

    text(
        2, 1,
        active == SNAKE ? "SNAKE" :
        active == PONG ? "BRICK" :
        active == FLAPPY ? "FLAPPY" : "RACE"
    );

    if (active == PONG || active == DODGE) {
        text(38, 1, "H:");

        view_render.print(
            active == PONG
                ? data.pong.lives
                : data.dodge.lives
        );
    }

    text(74, 1, "S:");
    view_render.print(score);

    view_render.drawLine(
        0, 10, 127, 10, WHITE);

    if (paused) {
        view_render.fillRect(
            14, 24, 100, 26, BLACK);

        view_render.drawRect(
            14, 24, 100, 26, WHITE);

        text(46, 27, "PAUSED");

        text(
            31, 39,
            active == FLAPPY
                ? "DOWN:RESUME"
                : "UP:RESUME"
        );
    }

    return true;
}
