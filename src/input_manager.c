#include "../headers/input_manager.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>

static struct termios old_termios; // Conserve l'état du terminal pour le restaurer à la fin

int fd_write_fifo;

void clean_ending()
{
    printf("Terminaison propre input_manager\n");
    close(fd_write_fifo);
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios); // Restauration terminal
    exit(EXIT_FAILURE);
}

void input_loop()
{
    char c;
    command_t cmd;
    struct termios new_termios;
    struct sigaction sa;

    // == Redéfinition du Ctrl C ==
    sa.sa_handler = clean_ending;
    sigaction(SIGINT, &sa, NULL);

    // == Configuration du terminal ==
    tcgetattr(STDIN_FILENO, &old_termios); // Garde l'état du terminal actuel comme base
    new_termios = old_termios;

    new_termios.c_lflag &= ~(ICANON | ECHO);        // Désactive la lecture char par char et n'affiche pas les touches tapées
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios); // Applique les modifs

    // Ouvre la fifo en écriture
    fd_write_fifo = open("2048_fifo", O_WRONLY);

    while (1)
    {
        c = getchar();

        if (c == 27)
        { // ESC
            char c1 = getchar();
            char c2 = getchar();

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
        else if (c == 'q')
        {
            cmd = CMD_QUIT;
        }
        else
        {
            printf("Entrée inconnue\n");
            continue;
        }

        write(fd_write_fifo, &cmd, sizeof(command_t));

        if (cmd == CMD_QUIT)
            clean_ending();
    }

    clean_ending();
}
