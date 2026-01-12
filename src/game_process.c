#include "../headers/game_process.h"
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

game_infos_t game_info;
int fd_cmd;
int fd_pipe_affichage[2];
int fork_res;

void game_process()
{

    pipe(fd_pipe_affichage);

    fork_res = fork();
    if (fork_res > 0)
    { // PERE (game_process)

        close(fd_pipe_affichage[0]); // Ferme la lecture

        pthread_t t_move, t_goal;
        command_t last_cmd;

        // == INIT de game_info ==
        for (size_t i = 0; i < GRID_SIZE; i++)
            for (size_t j = 0; j < GRID_SIZE; j++)
                game_info.grid[i][j] = 0;
        game_info.grid[1][1] = 2;
        game_info.score = 2;
        game_info.game_state = STATE_NOT_FINISHED;

        struct sigaction sa;
        sa.sa_handler = stop_handler;
        sigaction(SIGTERM, &sa, NULL);

        pthread_create(&t_move, NULL, move_and_score_loop, &game_info);
        pthread_create(&t_goal, NULL, goal_loop, &game_info);

        main_loop(&last_cmd);
    }
    else
    {                                // FILS (display)
        close(fd_pipe_affichage[1]); // Ferme l'écriture

        display_loop(fd_pipe_affichage[0]);
    }
}

void main_loop(command_t *last_cmd)
{
    fd_cmd = open("2048_fifo", O_RDONLY);
    while (1)
    {
        read(fd_cmd, last_cmd, sizeof(command_t)); // Récupère la cmd envoyée par input_manager
        kill(getpid(), SIGUSR1);                   // Notifie move_and_score
        if (*last_cmd == CMD_QUIT)
        {
            break;
        }
    }
    stop_handler(0);
}

void *move_and_score_loop(void *arg)
{
    game_infos_t *game_info = (game_infos_t *)arg;
    struct sigaction sa;
    sa.sa_handler = move_and_score_handler;
    sigaction(SIGUSR1, &sa, NULL);
    return NULL;
}

void move_and_score_handler(int sig)
{
    bool moved = false;
    command_t last_cmd;
    switch (last_cmd)
    {
    case CMD_UP:
        printf("Move UP\n");
        moved = move_up(&game_info);
        break;
    case CMD_DOWN:
        moved = move_down(&game_info);
        break;
    case CMD_LEFT:
        moved = move_left(&game_info);
        break;
    case CMD_RIGHT:
        moved = move_right(&game_info);
        break;
    default:
        return;
    }

    if (moved)
    {
        add_random_tile(&game_info);
    }

    kill(getpid(), SIGUSR2); // notif affichage + victoire
}

void *goal_loop(void *arg)
{
    game_infos_t *game_info = (game_infos_t *)arg;
    struct sigaction sa;
    sa.sa_handler = goal_handler;
    sigaction(SIGUSR2, &sa, NULL);
    goal_handler(0); // Premier affichage
    return NULL;
}

void goal_handler(int sig)
{
    bool complet = true;
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (game_info.grid[i][j] == 0)
            {
                complet = false;
            }
            else if (game_info.grid[i][j] >= 2048)
            {
                game_info.game_state = STATE_WIN;
                break;
            }
        }
    }
    if (complet && game_info.game_state == STATE_NOT_FINISHED)
    {
        game_info.game_state = STATE_LOSE;
    }

    write(fd_pipe_affichage[1], &game_info, sizeof(game_infos_t));

    if (game_info.game_state != STATE_NOT_FINISHED)
    {
        sleep(1);
        kill(getpid(), SIGTERM);
    }
}

void stop_handler(int sig)
{
    close(fd_cmd);
    close(fd_pipe_affichage[1]);

    kill(fork_res, SIGTERM);
    printf("Terminaison propre game process\n");
    exit(EXIT_SUCCESS);
}

static void compress_col_up(int grid[4][4], int col)
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

static bool merge_col_up(int grid[4][4], int col, int *score)
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

static void compress_col_down(int grid[4][4], int col)
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

static bool merge_col_down(int grid[4][4], int col, int *score)
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

static void compress_row_left(int row[4])
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

static bool merge_row_left(int row[4], int *score)
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

static void compress_row_right(int row[4])
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

static bool merge_row_right(int row[4], int *score)
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
