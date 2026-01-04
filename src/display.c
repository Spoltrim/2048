#include <stdio.h>
#include "../headers/display.h"

void display_game(const game_infos_t *infos)
{
    // Efface l'écran pour un affichage propre (optionnel)
    printf("\033[2J\033[H");

    printf("====== 2048 ======\n\n");
    printf("Score : %u\n\n", infos->score);

    // Affichage de la grille
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
            printf("🎉 Victoire ! Vous avez atteint 2048 !\n");
            break;
        case STATE_LOSE:
            printf("💀 Défaite ! Aucun coup possible.\n");
            break;
    }

    fflush(stdout);
}