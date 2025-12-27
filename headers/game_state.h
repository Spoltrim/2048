#pragma once

#include <cstdint>
#include <sys/types.h>

#define GRID_SIZE 4

typedef enum {
    STATE_NOT_FINISHED,
    STATE_LOSE,
    STATE_WIN
} game_state_t;

typedef struct game_infos_t {
    uint16_t grid[GRID_SIZE][GRID_SIZE];
    uint32_t score;
    game_state_t game_state;
}game_infos_t;