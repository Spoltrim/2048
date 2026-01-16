#include "../headers/input_manager.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>

static struct termios old_termios; // Conserve l'état du terminal pour le restaurer à la fin

int fd_write_fifo; // FD de la fifo en écriture vers game_process

void clean_ending()
{
    printf("Terminaison propre input_manager\n");
    close(fd_write_fifo);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios); // Restauration du terminal
    exit(EXIT_FAILURE);
}

void input_loop()
{
    // == Redéfinition du Ctrl C ==
    struct sigaction sa;
    sa.sa_handler = clean_ending;
    sigaction(SIGINT, &sa, NULL);

    // == Configuration du terminal ==
    // Par défaut, le terminal récupère stdin caractères par caractères
    // Ici, on peut récupérer plusieurs caractères en un (ex : < ESC[A > pour la flèche du haut )
    struct termios new_termios; // Pour configurer le nouveau terminal
    tcgetattr(STDIN_FILENO, &old_termios); // Garde l'état du terminal actuel comme base
    new_termios = old_termios;

    new_termios.c_lflag &= ~(ICANON | ECHO);        // Désactive la lecture char par char et n'affiche pas les touches tapées
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios); // Applique les modifs

    // Ouvre la fifo en écriture
    fd_write_fifo = open("2048_fifo", O_WRONLY);

    char c;
    command_t cmd;

    while (1)
    {
        c = getchar(); // Récupère le 1er caractère de l'entrée ( devrait être ESC )

        if (c == 27) // Si c'est ESC
        {
            char c1 = getchar(); // Récup le 2e ( devrait être [ )
            char c2 = getchar(); // Récup le 3e ( devrait être A à D )

            // Une flèche correspond à [A ou [B etc
            if (c1 == '[')
            {
                switch (c2)
                {
                case 'A':
                    cmd = CMD_UP;
                    break;
                case 'B':
                    cmd = CMD_DOWN;
                    break;
                case 'C':
                    cmd = CMD_RIGHT;
                    break;
                case 'D':
                    cmd = CMD_LEFT;
                    break;
                default:
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
        else if (c == 'q') // Si le caractère était juste 'q', on envoie la cmd pour quitter
        {
            cmd = CMD_QUIT;
        }
        else
        {
            printf("Entrée inconnue\n");
            continue;
        }

        // Envoie la commande par fifo à game_process
        write(fd_write_fifo, &cmd, sizeof(command_t));

        
        if (cmd == CMD_QUIT) {
            clean_ending();
        }
    }
}
