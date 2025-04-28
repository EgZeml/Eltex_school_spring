#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Запуск: %s <количество чисел для генерации>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int amount = atoi(argv[1]);
    if (amount <= 0) { 
        fprintf(stderr, "Количество должно быть >0\n"); 
        return 1; 
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) { 
        perror("pipe"); 
        return 1; 
    }

    // Подготовим набор сигналов для блокировки
    sigset_t sigset;
    // Очищаем
    sigemptyset(&sigset);
    // Добавляем сигналы в структуру
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGUSR2);
    // Блокируем, т.е. оставляем в очереди
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    pid_t pid = fork();
    if (pid == -1) { 
        perror("fork"); 
        return 1; 
    }

    if (pid == 0) {
        // ---------- Дочерний процесс ----------
        close(pipefd[0]);

        srand(time(NULL) ^ getpid());

        for (int i = 0; i < amount; ++i) {
            int val = rand();
            if (write(pipefd[1], &val, sizeof val) != sizeof val) {
                perror("child write"); 
                break;
            }

            int sig;
            // Ждем сигнал о блокировке файла 
            do { 
                sigwait(&sigset, &sig); 
            } while (sig != SIGUSR1);
            // Ждем сигнал о разблокировке файла 
            do { 
                sigwait(&sigset, &sig); 
            } while (sig != SIGUSR2);

            // Открываем файл для чтения
            int fd = open("output.dat", O_RDONLY);
            if (fd == -1) { 
                perror("child open"); 
                break; }

            int tmp;
            // Читаем последнее (новое) число в файле
            if (pread(fd, &tmp, sizeof tmp, i * sizeof(int)) == sizeof tmp)
                printf("Child read file[%d] = %d\n", i, tmp);
            else
                perror("child pread");

            close(fd);
        }
        close(pipefd[1]);
        return 0;
    } else {
        // ---------- Родительский процесс ---------- 
        close(pipefd[1]);

        int fd = open("output.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) { 
            perror("parent open"); 
            return 1; 
        }

        for (int i = 0; i < amount; ++i) {
            int num;
            if (read(pipefd[0], &num, sizeof num) != sizeof num) {
                perror("parent read"); 
                break;
            }
            printf("Parent received[%d] = %d\n", i, num);

            kill(pid, SIGUSR1);
            if (write(fd, &num, sizeof num) != sizeof num) {
                perror("parent write");
            }
            // Сбросить буфер в файл
            fsync(fd);
            kill(pid, SIGUSR2);
        }

        close(fd);
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        return 0;
    }
}