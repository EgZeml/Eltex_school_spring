#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Запуск: ./%s <количество чисел для генерации>", argv[0]);
        exit(EXIT_FAILURE);
    }

    int amount_of_numbers = atoi(argv[1]);
    if (amount_of_numbers <= 0) {
        fprintf(stderr, "Неверное количество чисел: %s", argv[1]);
        exit(EXIT_FAILURE);
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(pipefd[0]);
        
        srand(time(NULL));

        for (int i = 0; i < amount_of_numbers; i++) {
            int value = rand();
            if(write(pipefd[1], &value, sizeof(value)) != sizeof(value)) {
                perror("child write");
                break;
            }                    
        }

        close(pipefd[1]);
        exit (EXIT_FAILURE);
    } else {
        close(pipefd[1]);

        int fd = open("output.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd < 0) {
            perror("open");
            close(pipefd[0]);
            waitpid(pid, NULL, 0);
            exit(EXIT_FAILURE);
        }

        int tmp_number;
        for (int i = 0; i < amount_of_numbers; i++) {
            ssize_t r = read(pipefd[0], &tmp_number, sizeof(tmp_number));

            if (r == 0) {
                break;
            }

            if (r < 0) {
                perror("parent read");
                break;
            }

            printf("%d ", tmp_number);

            if (write(fd, &tmp_number, sizeof(tmp_number)) != sizeof(tmp_number)) {
                perror("file write");
                break;
            }
        }

        printf("\n");

        close(pipefd[0]);
        close(fd);

        waitpid(pid, NULL, 0);
        return EXIT_SUCCESS;
    }
}