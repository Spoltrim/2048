#include "../headers/input_manager.h"

static struct termios old_termios;

void input_loop() {
    int fd;
    char c;
    command_t cmd;

    struct termios new_termios;

    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;

    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    fd = open("/tmp/2048_fifo", O_WRONLY);

    while (1) {
        c = getchar();

        if (c == 27) { // ESC
            char seq1 = getchar();
            char seq2 = getchar();

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
            break;
    }

    close(fd);
}