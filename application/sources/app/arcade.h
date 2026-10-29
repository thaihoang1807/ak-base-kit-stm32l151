#ifndef AK_MINI_ARCADE_H
#define AK_MINI_ARCADE_H

#include "app.h"
#include "message.h"

// Tin hieu rieng cua task HELLO: giu MODE de ve menu.
enum {
    ARCADE_EXIT_SIG = AC_TASK_HELLO_TICK + 1
};

// true: arcade da xu ly; false: game ech xu ly tiep.
bool arcade_handle(ak_msg_t* msg);
bool arcade_render();

#endif
