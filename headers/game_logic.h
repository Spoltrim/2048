#pragma once

#include "game_state.h"

#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

bool move_up(game_infos_t *g);
bool move_down(game_infos_t *g);
bool move_left(game_infos_t *g);
bool move_right(game_infos_t *g);

void add_random_tile(game_infos_t* g);

void test_game_over(game_infos_t* g);