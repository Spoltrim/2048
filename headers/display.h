#pragma once
#include "game_state.h"
#include <stdlib.h>

void clean_display_ending(int sig);

void display_loop(int fd_lecture);

void display_game(const game_infos_t *infos);