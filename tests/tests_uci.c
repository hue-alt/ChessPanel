#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER 4096

int main(void) {
    int to_engine[2];
    int from_engine[2];
    pid_t pid;

    if (pipe(to_engine) == -1) {
        perror("pipe to engine");
        return EXIT_FAILURE;
    }
    if (pipe(from_engine) == -1) {
        perror("pipe from engine");
        return EXIT_FAILURE;
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        dup2(to_engine[0], STDIN_FILENO);
        dup2(from_engine[1], STDOUT_FILENO);

        close(to_engine[0]);
        close(to_engine[1]);
        close(from_engine[0]);
        close(from_engine[1]);

        execlp("stockfish", "stockfish", NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    close(to_engine[0]);
    close(from_engine[1]);

    char buffer[BUFFER];
    size_t readbytes;
    const char *cmd_uci = "uci\n";
    write(to_engine[1], cmd_uci, strlen(cmd_uci));

    while ((readbytes = read(from_engine[0], buffer, BUFFER - 1)) > 0) {
        buffer[readbytes] = '\0';
        printf("%s", buffer);
        if (strstr(buffer, "uciok") != NULL) {
            printf("\nuciok received, communication is working correctly.\n");
            break;
        }
    }
    const char *cmd_quit = "quit\n";
    write(to_engine[1], cmd_quit, strlen(cmd_quit));
    close(to_engine[1]);
    close(from_engine[0]);
    wait(NULL);
    return EXIT_SUCCESS;
}