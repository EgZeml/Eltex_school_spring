#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <semaphore.h>

static const char *SEM_C2P = "/sem_c2p";  // Ребенок → родитель
static const char *SEM_P2C = "/sem_p2c";  // Родитель → ребенок

// С помощью двух семафоров организовываем блокировку и доступ к файлу
int main(int argc, char *argv[]) {
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

    // 1) Создаем два семафора:
    // Первый следит за тем, что число находится в потоке и родитель может его забрать
    // Второе следит за тем, что родитель записал число в файл и дочерний может работать с файлом

    // 1.1) Создаем семафор для слежения за готовностью числа
    sem_t *sem_c2p = sem_open(SEM_C2P, O_CREAT | O_EXCL, 0666, 0);
    if (sem_c2p == SEM_FAILED && errno == EEXIST) {
        sem_c2p = sem_open(SEM_C2P, 0);
    }
    if (sem_c2p == SEM_FAILED) {
        perror("sem_open sem_c2p");
        return EXIT_FAILURE;
    }

    // 1.2) Создаем семафор для слежения за готовностью файла
    sem_t *sem_p2c = sem_open(SEM_P2C, O_CREAT | O_EXCL, 0666, 0);
    if (sem_p2c == SEM_FAILED && errno == EEXIST) {
        sem_p2c = sem_open(SEM_P2C, 0);
    }
    if (sem_p2c == SEM_FAILED) {
        perror("sem_open sem_p2c");
        // Удаляем открый ранее семафор
        sem_close(sem_c2p);
        sem_unlink(SEM_C2P);
        return EXIT_FAILURE;
    }

    // 2) Создаем дочерний прочесс
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

            // 3) С помощью семафоров сообщаем о готовности числа 
            // 3.1) Сообщаем родителю, что число готово
            if (sem_post(sem_c2p) < 0) {
                perror("child sem_post(c2p)");
                break;
            }

            // 3.2) Ждём, пока родитель запишет в файл
            if (sem_wait(sem_p2c) < 0) {
                perror("child sem_wait(p2c)");
                break;
            }
        
            // Открываем файл для чтения
            int fd = open("output.dat", O_RDONLY);
            if (fd == -1) { 
                perror("child open"); 
                break; }

            int tmp;
            // Читаем последнее (новое) число в файле
            ssize_t r = pread(fd, &tmp, sizeof tmp, i * sizeof tmp);
            if (r == sizeof tmp) {
                printf("Child read file[%d] = %d\n", i, tmp);
            } else if (r == 0) {
                fprintf(stderr, "Child: файл ещё не содержит данных на позиции %d\n", i);
            } else {
                perror("child pread");
            }
                        
            close(fd);
        }
        close(pipefd[1]);
        sem_close(sem_c2p);
        sem_close(sem_p2c);
        return EXIT_SUCCESS;
    } else {
        // ---------- Родительский процесс ---------- 
        close(pipefd[1]);

        int fd = open("output.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) { 
            perror("parent open"); 
            return 1; 
        }

        for (int i = 0; i < amount; ++i) {
            // 4) С помощью семафоров получаем момент, когда число готово и записываем его в файл
            // 4.1) Ждём от ребёнка готовности числа
            if (sem_wait(sem_c2p) < 0) {
                perror("parent sem_wait(c2p)");
                break;
            }
            int num;
            if (read(pipefd[0], &num, sizeof num) != sizeof num) {
                perror("parent read"); 
                break;
            }
            printf("Parent received[%d] = %d\n", i, num);
            if (write(fd, &num, sizeof num) != sizeof num) {
                perror("parent write");
            }
            // Сбросить буфер в файл
            fsync(fd);
            // 4.2) Сигнал ребенку, что файл обновлён
            if (sem_post(sem_p2c) < 0) {
                perror("parent sem_post(p2c)");
                break;
            }
        }
        close(fd);
        close(pipefd[0]);
        waitpid(pid, NULL, 0);

        sem_close(sem_c2p);
        sem_close(sem_p2c);
        sem_unlink(SEM_C2P);
        sem_unlink(SEM_P2C);
        return EXIT_SUCCESS;
    }
}