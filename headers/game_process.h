#pragma once
#include "game_state.h"
#include "commands.h"
#include "display.h"

#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

void game_process();

void main_loop(command_t *last_cmd);

void *move_and_score_loop(void *arg);

void move_and_score_handler(int sig);

void *goal_loop(void *arg);

void goal_handler(int sig);

void stop_handler(int sig);

bool move_up(game_infos_t *g);
bool move_down(game_infos_t *g);
bool move_left(game_infos_t *g);
bool move_right(game_infos_t *g);