#include <signal.h>
#include <stdio.h>
#include "../headers/display.h"
#include <unistd.h>
#include <math.h>

int fd; // Stocke le FD du pipe anonyme depuis le thread goal pour pouvoir le fermer depuis le handler

void clean_display_ending(int sig)
{
    (void)sig; // Enleve le unsused warning
    close(fd);
    printf("Terminaison propre display\n");
    exit(EXIT_SUCCESS);
}

void display_loop(int fd_lecture)
{
    fd = fd_lecture;

    // == Def de la fonction de terminaison propre à l'arrivée du SIGINT (envoyé par game_process ou Ctrl C) ==
    struct sigaction sa;
    sa.sa_handler = clean_display_ending;
    sigaction(SIGINT, &sa, NULL);

    game_infos_t game_info; // Buffer pour récupérer les infos du read
    while (1)
    {
        // Attend et lit les données envoyées par le thread goal, stocke dans game_info
        ssize_t r = read(fd, &game_info, sizeof(game_infos_t));
        if (r <= 0) {
            fprintf(stderr,"Erreur read display\n");
            break;
        }
        display_game(&game_info); // Appelle la fonction qui affiche la grille
    }
    clean_display_ending(0);
}


static void set_bg_color_for_value(uint16_t value)
{
    // Transforme 2-4-8-16 en 1-2-3-4
    double max_power = log2(2048);
    double power = log2(value); 

    // Récup un coef (0 - 1) correspondant à 2 - 2048
    double t = (power - 1.0) / (max_power - 1.0);
    //if (t < 0) t = 0;
    //if (t > 1) t = 1;

    int r = 255;
    int g = (int)(220 * (1.0 - t));
    int b = 0;

    printf("\033[48;2;%d;%d;%dm", r, g, b);
}

static void reset_bg_color()
{
    printf("\033[0m");
}



void display_game(const game_infos_t *infos)
{
    // ctrl L pour afficher seuleument une grille
    printf("\033[2J\033[H");

    // affichage du jeu
    printf("====== 2048 ======\n\n");
    printf("Score : %u\n\n", infos->score);

    // affichage de la grille
    for (int i = 0; i < GRID_SIZE; i++)
    {
        printf("+------+------+------+------+ \n");
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (infos->grid[i][j] == 0)
            {
                printf("| %4s ", " ");
            }
            else
            {
                printf("|");
                set_bg_color_for_value(infos->grid[i][j]);
                printf(" %4u ", infos->grid[i][j]);
                reset_bg_color();
            }
        }
        printf("|\n");
    }
    printf("+------+------+------+------+\n\n");

    // état du jeu
    switch (infos->game_state)
    {
    case STATE_NOT_FINISHED:
        printf("Partie en cours...\n");
        break;
    case STATE_WIN:
        printf("Victoire! Vous avez atteint 2048 !\n");
        break;
    case STATE_LOSE:
        printf("Défaite! Aucun coup possible.\n");
        break;
    }
    printf("Appuyer sur Q pour quitter.\n");

    fflush(stdout);
}