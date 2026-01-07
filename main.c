#include "headers/input_manager.h"
#include "headers/game_process.h"
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    
    mkfifo("2048_fifo",S_IWUSR | S_IRUSR); // Créé la fifo

    int fork_res = fork();
    if (fork_res > 0) 
    { // PERE
        input_loop();
    }
    else 
    { // FILS 
        game_process();
    }

    return EXIT_SUCCESS;
}