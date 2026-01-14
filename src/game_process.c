#include "../headers/game_process.h"
#include <bits/pthreadtypes.h>
#include <bits/types/sigset_t.h>
#include <pthread.h>
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
command_t last_cmd;
pthread_t t_move, t_goal;

pthread_mutex_t mutex;

void game_process()
{

    pipe(fd_pipe_affichage);

    fork_res = fork();
    if (fork_res > 0)
    { // PERE (game_process)

        close(fd_pipe_affichage[0]); // Ferme la lecture

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

        pthread_t t_main;
        pthread_create(&t_main, NULL, main_loop, &last_cmd);
    }
    else
    {                                // FILS (display)
        close(fd_pipe_affichage[1]); // Ferme l'écriture

        display_loop(fd_pipe_affichage[0]);
    }
}

void *main_loop(void *arg)
{
    command_t *last_cmd = (command_t *)arg;

    fd_cmd = open("2048_fifo", O_RDONLY);
    
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    

    while (1)
    {
        int r = read(fd_cmd, last_cmd, sizeof(command_t)); // Récupère la cmd envoyée par input_manager
        printf("Testtt %d\n", r);
        if (r < 0)
        {
            break;
        }
        pthread_kill(t_move, SIGUSR1); // Notifie move_and_score
        if (*last_cmd == CMD_QUIT)
        {
            break;
        }
    }
    stop_handler(0);
    return NULL;
}

void *move_and_score_loop(void *arg)
{
    printf("debut move_and_score_loop\n");
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR2); // Bloque uniquement SIGUSR2
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    game_infos_t *game_info = (game_infos_t *)arg;
    struct sigaction sa;
    sa.sa_handler = move_and_score_handler;
    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    printf("Signal handler for SIGUSR1 registered\n");
    while(1) {
        pause();
    }
    return NULL;
}

void move_and_score_handler(int sig)
{
    printf("Move and score handler called\n");
    bool moved = false;

    pthread_mutex_lock(&mutex);

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
    pthread_mutex_unlock(&mutex);

    kill(getpid(), SIGUSR2); // notif affichage + victoire
}

void *goal_loop(void *arg)
{
    /*
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    */

    game_infos_t *game_info = (game_infos_t *)arg;
    struct sigaction sa;
    sa.sa_handler = goal_handler;
    sigaction(SIGUSR2, &sa, NULL);
    goal_handler(0); // Premier affichage
    while(1) {
        pause();
    }
    return NULL;
}

void goal_handler(int sig)
{
    printf("Goal handler\n");
    pthread_mutex_lock(&mutex);


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
    
    pthread_mutex_unlock(&mutex);

    if (game_info.game_state != STATE_NOT_FINISHED)
    {
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

