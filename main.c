#include "headers/commands.h"
#include "stdlib.h"
#include "headers/input_manager.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    
    mkfifo("2048_fifo",S_IWUSR | S_IRUSR); // Créé la fifo

    if (fork() > 0) 
    { // PERE
        input_loop();
    }
    else 
    { // FILS 
        // TEST POUR LIRE LES COMMANDES (à remplacer par le lancement de la logique)
        int fd = open("2048_fifo", O_RDONLY);
        while(1) {
            command_t cmd;
            read(fd,&cmd,sizeof(command_t));
            printf("%u\n",cmd);
            if ( cmd == CMD_QUIT) {
                break;
            }
        }
    }



    return EXIT_SUCCESS;
}