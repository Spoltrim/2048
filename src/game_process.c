#include "../headers/game_process.h"
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

game_infos_t game_info;
int fd_pipe_affichage[2];

void game_process() {

    pipe(fd_pipe_affichage);

    int fork_res = fork();
    if (fork_res > 0) 
    { // PERE (game_process)

        close(fd_pipe_affichage[0]); // Ferme la lecture

        pthread_t t_move, t_goal;
        command_t last_cmd;

        // == INIT de game_info ==
        for (size_t i = 0; i<GRID_SIZE; i++) 
            for (size_t j = 0; i<GRID_SIZE; i++) 
                game_info.grid[i][j] = 0;
        game_info.grid[1][1] = 2;
        game_info.score = 2;
        game_info.game_state = STATE_NOT_FINISHED;


        pthread_create(&t_move, NULL, move_and_score_loop, &game_info);
        pthread_create(&t_goal, NULL, goal_loop, &game_info);

        main_loop(fd_pipe_affichage[1],&last_cmd);
    }
    else 
    { // FILS (display)
        close(fd_pipe_affichage[1]); // Ferme l'écriture

        display_loop(fd_pipe_affichage[0]);
    }
}


void main_loop(int fd_pipe_cmd, command_t* last_cmd) {
    int fd = open("2048_fifo", O_RDONLY);
    while(1) {
        read(fd,last_cmd,sizeof(command_t));
        printf("%u\n",*last_cmd);
        kill(getpid(),SIGUSR1);
        if ( *last_cmd == CMD_QUIT) {
            break;
        }
    }
    close(fd);
    printf("Bonne terminaison lecteur\n");

    close(fd_pipe_cmd);
}



void* move_and_score_loop(void* arg) {
    game_infos_t* game_info = (game_infos_t*)arg;
    struct sigaction sa;
    sa.sa_handler = move_and_score_handler;
    sigaction(SIGUSR1, &sa, NULL);
    return NULL;
}

void move_and_score_handler(int sig) {
    game_info.grid[1][1] *=2;
    game_info.score*=2;
    kill(getpid(),SIGUSR2);
}



void* goal_loop(void* arg) {
    game_infos_t* game_info = (game_infos_t*)arg;
    struct sigaction sa;
    sa.sa_handler = goal_handler;
    sigaction(SIGUSR2, &sa, NULL);
    goal_handler(0); // Premier affichage
    return NULL;
}

void goal_handler(int sig) {
    if (game_info.game_state == STATE_LOSE) {
        kill(getpid(),SIGINT);
    }
    write(fd_pipe_affichage[1],&game_info,sizeof(game_infos_t));
}