#include <stdio.h>
#include "../headers/display.h"
#include <unistd.h>

void display_loop(int fd_lecture) {
    while(1) {
        game_infos_t game_info;
        read(fd_lecture,&game_info,sizeof(game_infos_t));
        display_game(&game_info);
    }
    close(fd_lecture);
}

void display_game(const game_infos_t *infos)
{
    //ctrl L pour afficher seuleument une grille
    printf("\033[2J\033[H");

    //affichage du jeu
    printf("====== 2048 ======\n\n");
    printf("Score : %u\n\n", infos->score);

    //affichage de la grille 
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("+------+------+------+------+ \n");
        for (int j = 0; j < GRID_SIZE; j++) {
            if (infos->grid[i][j] == 0) {
                printf("| %4s ", " ");
            } else {
                printf("| %4u ", infos->grid[i][j]);
            }
        }
        printf("|\n");
    }
    printf("+------+------+------+------+\n\n");

    //état du jeu
    switch (infos->game_state) {
        case STATE_NOT_FINISHED:
            printf("Partie en cours...\n");
            break;
        case STATE_WIN:
            printf("🎉 Victoire! Vous avez atteint 2048 !\n");
            break;
        case STATE_LOSE:
            printf("💀 Défaite! Aucun coup possible.\n");
            break;
    }

    fflush(stdout);
}