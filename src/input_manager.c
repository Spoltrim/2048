#include "../headers/input_manager.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

static struct termios old_termios; // Conserve l'état du terminal pour le restaurer à la fin

void input_loop() {
    int fd;
    char c;
    command_t cmd;

    struct termios new_termios;

    tcgetattr(STDIN_FILENO, &old_termios); // Garde l'état du terminal actuel comme base
    new_termios = old_termios;

    new_termios.c_lflag &= ~(ICANON | ECHO); // Désactive la lecture char par char et n'affiche pas les touches tapées
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios); // Applique les modifs

    fd = open("2048_fifo", O_WRONLY); // Ouvre la fifo en écriture
    
    while (1) {
        c = getchar();

        if (c == 27) { // ESC
            char seq1 = getchar(); 
            char seq2 = getchar();

            // Une flèche correspond à [A ou [B etc
            if (seq1 == '[') {
                switch (seq2) {
                    case 'A': cmd = CMD_UP; break;
                    case 'B': cmd = CMD_DOWN; break;
                    case 'C': cmd = CMD_RIGHT; break;
                    case 'D': cmd = CMD_LEFT; break;
                    default: continue;
                }
            } else {
                continue;
            }
        }
        else if (c == 'q') {
            cmd = CMD_QUIT;
        }
        else {
            continue;
        }

        write(fd, &cmd, sizeof(command_t));

        if (cmd == CMD_QUIT)
            printf("Il veut quitter\n");
    }

    close(fd);
}