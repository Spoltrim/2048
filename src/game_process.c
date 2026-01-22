#include "../headers/game_process.h"
#include <unistd.h>

game_infos_t game_info; // Infos générales de la partie, partagé par les threads de game_process
int fd_cmd; // FD de la FIFO ouverte en écriture vers input_manager
int fd_pipe_affichage[2]; // FDs du pipe anonyme entre processus display et le thread goal
int fork_res; // Résultat du fork pour garder le pid du processus display
command_t last_cmd; // Dernière commande jouée, récupérée par le thread MAIN et partagé avec ses threads
pthread_t t_move, t_goal; // Les id des threads

pthread_mutex_t mutex_game_info, mutex_last_cmd; // Servent pour protéger game_info et last_cmd contre la concurrence d'accès par les threads

// Permettent aux boucles des threads de savoir quand s'executer (à l'arrivée d'un signal)
// Pour éviter d'avoir beaucoup de code dans un handler
volatile sig_atomic_t atomic_move = 0, atomic_goal = 0; 

void game_process()
{
    // Crée le pipe anonyme entre goal et display
    pipe(fd_pipe_affichage);

    // Init les mutex (besoin)
    pthread_mutex_init(&mutex_game_info, NULL);
    pthread_mutex_init(&mutex_last_cmd, NULL);


    fork_res = fork();
    if (fork_res > 0)
    { // == PERE (game_process) ==

        close(fd_pipe_affichage[0]); // Ferme la lecture

        // == INIT de game_info ==
        for (size_t i = 0; i < GRID_SIZE; i++)
            for (size_t j = 0; j < GRID_SIZE; j++)
                game_info.grid[i][j] = 0;
        add_random_tile(&game_info);
        add_random_tile(&game_info);
        game_info.score = 0;
        game_info.game_state = STATE_NOT_FINISHED;

        // == Def de la fonction d'arret du programme dès SIGINT (envoyé avec Ctrl C ou par des threads)
        struct sigaction sa;
        sa.sa_handler = stop_handler;
        sigaction(SIGINT, &sa, NULL);

        // == Crée les threads move_and_score et goal ==
        pthread_create(&t_move, NULL, move_and_score_loop, &game_info);
        pthread_create(&t_goal, NULL, goal_loop, &game_info);

        main_loop(&last_cmd);
    }
    else
    { // == FILS (display) ==
        close(fd_pipe_affichage[1]); // Ferme l'écriture

        display_loop(fd_pipe_affichage[0]);
    }
}

// Thread MAIN : ouvre la FIFO et attend les commandes envoyées par input_manager, la place dans last_cmd et notifie move_and_score
void *main_loop(void *arg)
{
    command_t *last_cmd = (command_t *)arg; // Converti l'argument

    // Ouvre la fifo en lecture
    fd_cmd = open("2048_fifo", O_RDONLY);
    
    command_t buffer;
    while (1)
    {
        // Attend et lit les commandes envoyées par input_manager, la place dans buffer
        int r = read(fd_cmd, &buffer, sizeof(command_t)); // Récupère la cmd envoyée par input_manager

        // Assigne last_cmd avec une protection à la concurrence d'accès aux ressources
        pthread_mutex_lock(&mutex_last_cmd);
        *last_cmd = buffer;
        pthread_mutex_unlock(&mutex_last_cmd);

        if (r <= 0) // Si erreur ou fifo fermée on quitte
        {
            break;
        }
        pthread_kill(t_move, SIGUSR1); // Notifie move_and_score d'une nouvelle commande
        if (buffer == CMD_QUIT)
        {
            break;
        }
    }
    stop_handler(0);
    return NULL;
}

void *move_and_score_loop(void *arg)
{
    game_infos_t *game_info = (game_infos_t *)arg; // Converti aussi l'argument

    // == Def le handler à l'arrivée de SIGUSR1 (envoyé par thread MAIN) ==
    struct sigaction sa;
    sa.sa_handler = move_and_score_handler;
    sigaction(SIGUSR1, &sa, NULL);

    while(1) {
        if (!atomic_move) { // Si SIGUSR1 n'a pas été appelé depuis, on laisse tourner le thread
            usleep(1000); // Pour pas qu'il tourne trop vite sans rien faire
            continue;
        }

        bool moved = false;

        // Réquisitionne game_info et last_cmd pendant son utilisation (contre concurrence de l'accès aux ressources)
        pthread_mutex_lock(&mutex_game_info);
        pthread_mutex_lock(&mutex_last_cmd);

        // Effectue le mouvement selon la cmd
        switch (last_cmd)
        {
            case CMD_UP:
                moved = move_up(game_info);
                break;
            case CMD_DOWN:
                moved = move_down(game_info);
                break;
            case CMD_LEFT:
                moved = move_left(game_info);
                break;
            case CMD_RIGHT:
                moved = move_right(game_info);
                break;
            default:
                break;
        }

        pthread_mutex_unlock(&mutex_last_cmd); // Libère last_cmd (plus besoin)

        // Si le coup était valide, on ajoute une nouvelle tuile
        if (moved)
        {
            add_random_tile(game_info);
        }

        pthread_mutex_unlock(&mutex_game_info); // Libère game_info

        pthread_kill(t_goal, SIGUSR2); // Notifie le thread goal d'une action

        atomic_move = 0; // Signal traité
    }
    return NULL;
}

void move_and_score_handler(int sig)
{
    (void)sig; // Enleve le unsused warning
    atomic_move = 1; // SIgnal à traiter dans move_and_score_loop
}



void *goal_loop(void *arg)
{
    game_infos_t *game_info = (game_infos_t *)arg; // Converti l'argument

    // == Def le handler à l'arrivée de SIGUSR2 (envoyé par thread move and score)
    struct sigaction sa;
    sa.sa_handler = goal_handler;
    sigaction(SIGUSR2, &sa, NULL);

    goal_handler(0); // Fait un premier affichage de la grille au lancement

    while(1) {
        if (!atomic_goal) {// Si SIGUSR2 n'a pas été appelé depuis, on laisse tourner le thread
            usleep(1000); // Pour pas qu'il tourne trop vite sans rien faire
            continue;
        }

        pthread_mutex_lock(&mutex_game_info); // Réquisitionne game_info pendant son accès

        test_game_over(game_info); // Teste la victoire ou défaite (rempli dans game_info.game_state)

        write(fd_pipe_affichage[1], game_info, sizeof(game_infos_t));
        
        pthread_mutex_unlock(&mutex_game_info); // Plus besoin de game_info

        // Si la partie est finie, on envoie SIGINT à input_manager et soi même pour terminer proprement le programme
        if (game_info->game_state != STATE_NOT_FINISHED) 
        {
            usleep(1000);
            kill(getpid(), SIGINT);
            kill(getppid(),SIGINT);
        }

        atomic_goal = 0; // Signal traité
    }
    return NULL;
}

void goal_handler(int sig)
{
    (void)sig; // Enleve le unsused warning
    atomic_goal = 1; // Signal à traiter dans goal_loop
}



void stop_handler(int sig)
{
    (void)sig; // Enleve le unsused warning

    close(fd_cmd);
    close(fd_pipe_affichage[1]);

    kill(fork_res, SIGINT); // Envoie SIGINT au processus display pour terminaison propre

    printf("Terminaison propre game process\n");
    exit(EXIT_SUCCESS);
}

