#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

#define MAX_MESSAGE 512

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0) {
        perror("tcsetattr()");
    }
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0) {
        perror("tcsetattr ICANON");

    }
    if (read(0, &buf, 1) < 0) {
        perror ("read()");
    }
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0) {
        perror ("tcsetattr ~ICANON");
    }
    return (buf);
}


int main() {
    int pid;
    int fd[2];
    pipe(fd);
    // fork the current process.
    if ((pid = fork()) < 0) {
        write(STDERR_FILENO, "Error in system call.\n", MAX_MESSAGE);
        return -1;
    }
    // child's process.
    if (pid == 0) {
        dup2(fd[0], STDIN_FILENO);
        // execute the tetris program.
        char* args[4] = {"./draw.out", NULL};
        if ((execvp(args[0], args) < 0)) {
            write(STDERR_FILENO, "Error in system call.\n", MAX_MESSAGE);
        }
    // father's process.
    } else {
        while (1) {
            char c = getch();
            // if user pressed "quit" key, quit the game by killing child process.
            if (c == 'q') {
                kill(pid, SIGKILL);
                system("clear");
                return 0;
            }
            // check if c is a valid game key. if not, ignore.
            if ((c != 'd') && (c != 'a') && (c != 's') && (c != 'w')) {
                continue;
            }
            // if the key is a valid game key (and not 'q'), pass it through pipe.
            if (write(fd[1], &c, sizeof(char)) < 0 ) {
                write(STDERR_FILENO, "Error in system call.\n", MAX_MESSAGE);
            }
            kill(pid, SIGUSR2);
        }
    }
    return 0;

}