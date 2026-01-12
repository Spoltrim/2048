#include "../headers/game_process.h"
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

game_infos_t game_info;
int fd_cmd;
int fd_pipe_affichage[2];
int fork_res;

void game_process() {

    pipe(fd_pipe_affichage);

    fork_res = fork();
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

        struct sigaction sa;
        sa.sa_handler = stop_handler;
        sigaction(SIGTERM, &sa, NULL);
        sigaction(SIGINT, &sa, NULL);


        pthread_create(&t_move, NULL, move_and_score_loop, &game_info);
        pthread_create(&t_goal, NULL, goal_loop, &game_info);

        main_loop(&last_cmd);
    }
    else 
    { // FILS (display)
        close(fd_pipe_affichage[1]); // Ferme l'écriture

        display_loop(fd_pipe_affichage[0]);
    }
}


void main_loop(command_t* last_cmd) {
    fd_cmd = open("2048_fifo", O_RDONLY);
    while(1) {
        read(fd_cmd,last_cmd,sizeof(command_t)); // Récupère la cmd envoyée par input_manager
        kill(getpid(),SIGUSR1); // Notifie move_and_score
        if ( *last_cmd == CMD_QUIT) {
            break;
        }
    }
    stop_handler(0);
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

    kill(getpid(),SIGUSR2); // Notifie goal
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
    bool complet = true;
    for (int i=0; i<GRID_SIZE; i++) {
        for (int j=0; j<GRID_SIZE; j++) {
            if (game_info.grid[i][j] == 0) {
                complet = false;
            } else if (game_info.grid[i][j] >= 2048) {
                game_info.game_state = STATE_WIN; break;
            }
        }
    }
    if (complet && game_info.game_state == STATE_NOT_FINISHED) {
        game_info.game_state = STATE_LOSE;
    }
    
    write(fd_pipe_affichage[1],&game_info,sizeof(game_infos_t));

    if (game_info.game_state != STATE_NOT_FINISHED) {
        sleep(1);
        kill(getpid(),SIGTERM);
    }
}


void stop_handler(int sig) {
    close(fd_cmd);
    close(fd_pipe_affichage[1]);

    kill(getppid(),SIGINT);
    kill(fork_res,SIGTERM);
    printf("Terminaison propre game process\n");
    exit(EXIT_SUCCESS);
}