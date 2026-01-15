#include "../headers/game_logic.h"
#include <string.h>
#include <stdint.h>

static void compress_col_up(uint16_t grid[4][4], int col)
{
    int tmp[4] = {0};
    int k = 0;

    for (int i = 0; i < 4; i++)
    {
        if (grid[i][col] != 0)
        {
            tmp[k++] = grid[i][col];
        }
    }

    for (int i = 0; i < 4; i++)
    {
        grid[i][col] = tmp[i];
    }
}

static bool merge_col_up(uint16_t grid[4][4], int col, uint32_t *score)
{
    bool merged = false;

    for (int i = 0; i < 3; i++)
    {
        if (grid[i][col] != 0 && grid[i][col] == grid[i + 1][col])
        {
            grid[i][col] *= 2;
            *score += grid[i][col];
            grid[i + 1][col] = 0;
            merged = true;
        }
    }
    return merged;
}

bool move_up(game_infos_t *g)
{
    bool moved = false;
    
        for (int col = 0; col < GRID_SIZE; col++)
        {

            int before[4];
            for (int i = 0; i < 4; i++)
                before[i] = g->grid[i][col];

            compress_col_up(g->grid, col);
            if (merge_col_up(g->grid, col, &g->score))
                moved = true;
            compress_col_up(g->grid, col);

            for (int i = 0; i < 4; i++)
            {
                if (before[i] != g->grid[i][col])
                {
                    moved = true;
                    break;
                }
            }
        }
        
    return moved;
}

static void compress_col_down(uint16_t grid[4][4], int col)
{
    int tmp[4] = {0};
    int k = 3;

    for (int i = 3; i >= 0; i--)
    {
        if (grid[i][col] != 0)
        {
            tmp[k--] = grid[i][col];
        }
    }

    for (int i = 0; i < 4; i++)
    {
        grid[i][col] = tmp[i];
    }
}

static bool merge_col_down(uint16_t grid[4][4], int col, uint32_t *score)
{
    bool merged = false;

    for (int i = 3; i > 0; i--)
    {
        if (grid[i][col] != 0 && grid[i][col] == grid[i - 1][col])
        {
            grid[i][col] *= 2;
            *score += grid[i][col];
            grid[i - 1][col] = 0;
            merged = true;
        }
    }
    return merged;
}

bool move_down(game_infos_t *g)
{
    bool moved = false;

    for (int col = 0; col < GRID_SIZE; col++)
    {

        int before[4];
        for (int i = 0; i < 4; i++)
            before[i] = g->grid[i][col];

        compress_col_down(g->grid, col);
        if (merge_col_down(g->grid, col, &g->score))
            moved = true;
        compress_col_down(g->grid, col);

        for (int i = 0; i < 4; i++)
        {
            if (before[i] != g->grid[i][col])
            {
                moved = true;
                break;
            }
        }
    }

    return moved;
}

static void compress_row_left(uint16_t row[4])
{
    int tmp[4] = {0};
    int k = 0;

    for (int i = 0; i < 4; i++)
    {
        if (row[i] != 0)
        {
            tmp[k++] = row[i];
        }
    }

    for (int i = 0; i < 4; i++)
    {
        row[i] = tmp[i];
    }
}

static bool merge_row_left(uint16_t row[4], uint32_t *score)
{
    bool merged = false;

    for (int i = 0; i < 3; i++)
    {
        if (row[i] != 0 && row[i] == row[i + 1])
        {
            row[i] *= 2;
            *score += row[i];
            row[i + 1] = 0;
            merged = true;
        }
    }

    return merged;
}

bool move_left(game_infos_t *g)
{
    bool moved = false;

    for (int i = 0; i < GRID_SIZE; i++)
    {

        int before[4];
        for (int j = 0; j < 4; j++)
            before[j] = g->grid[i][j];

        compress_row_left(g->grid[i]);
        if (merge_row_left(g->grid[i], &g->score))
            moved = true;
        compress_row_left(g->grid[i]);

        for (int j = 0; j < 4; j++)
        {
            if (before[j] != g->grid[i][j])
            {
                moved = true;
                break;
            }
        }
    }

    return moved;
}

static void compress_row_right(uint16_t row[4])
{
    int tmp[4] = {0};
    int k = 3;

    for (int i = 3; i >= 0; i--)
    {
        if (row[i] != 0)
        {
            tmp[k--] = row[i];
        }
    }

    for (int i = 0; i < 4; i++)
    {
        row[i] = tmp[i];
    }
}

static bool merge_row_right(uint16_t row[4], uint32_t *score)
{
    bool merged = false;

    for (int i = 3; i > 0; i--)
    {
        if (row[i] != 0 && row[i] == row[i - 1])
        {
            row[i] *= 2;
            *score += row[i];
            row[i - 1] = 0;
            merged = true;
        }
    }

    return merged;
}

bool move_right(game_infos_t *g)
{
    bool moved = false;

    for (int i = 0; i < GRID_SIZE; i++)
    {

        int before[4];
        for (int j = 0; j < 4; j++)
            before[j] = g->grid[i][j];

        compress_row_right(g->grid[i]);
        if (merge_row_right(g->grid[i], &g->score))
            moved = true;
        compress_row_right(g->grid[i]);

        for (int j = 0; j < 4; j++)
        {
            if (before[j] != g->grid[i][j])
            {
                moved = true;
                break;
            }
        }
    }

    return moved;
}

void add_random_tile(game_infos_t *g)
{
    int empty[GRID_SIZE * GRID_SIZE][2];
    int count = 0;

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (g->grid[i][j] == 0)
            {
                empty[count][0] = i;
                empty[count][1] = j;
                count++;
            }
        }
    }

    if (count == 0)
        return;

    int r = rand() % count;
    int value = (rand() % 10 == 0) ? 4 : 2;

    g->grid[empty[r][0]][empty[r][1]] = value;
}



static bool can_move(game_infos_t *g)
{
    game_infos_t tmp;

    memcpy(&tmp, g, sizeof(game_infos_t));

    if (move_up(&tmp) || move_down(&tmp) || move_left(&tmp) || move_right(&tmp)) 
        return true;

    return false;
}




void test_game_over(game_infos_t *g)
{
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (g->grid[i][j] >= 2048)
            {
                g->game_state = STATE_WIN;
                break;
            }
        }
    }
    if (!can_move(g) && g->game_state == STATE_NOT_FINISHED)
    {
        g->game_state = STATE_LOSE;
    }
}