#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};


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

    key_t key = ftok("./src/main.c", 'A'); // Генерируем ключ
    if (key == -1) {
        perror("ftok");
        return EXIT_FAILURE;
    }

    // 1) Создаем семафор для контроля за файлом
    // 1.1) Создаем семафор
    int semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid == -1) {
        if (errno == EEXIST) {
            /* Если уже есть — просто откроем */
            semid = semget(key, 1, 0);
        }
        if (semid == -1) {
            perror("semget");
            return EXIT_FAILURE;
        }
    }

    // 1.2) Начальное значение ресурса ноль
    union semun arg; 
    arg.val = 0;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL");
        return EXIT_FAILURE;
    }

    // 1.3) Структуры для управления ресурсом
    struct sembuf sem_P = {0, -1, 0};
    struct sembuf sem_V = {0, +1, 0};

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

            // 2) Сообщаем, что число в потоке и ожидаем сигнал о доступе к файлу
            // 2.1) Сигналим родителю: число готово
            if (semop(semid, &sem_V, 1) == -1) {
                perror("child semop V");
                break;
            }

            // 2.2) Ждём, пока родитель запишет в файл и даст доступ к нему
            if (semop(semid, &sem_P, 1) == -1) {
                perror("child semop P");
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
            // 3) Получаем сигнал о наличие числа в потоке и по окончанию открываем доступ к файлу 
            // 3.1) Ждём сигнала от ребёнка
            if (semop(semid, &sem_P, 1) == -1) {
                perror("parent semop P");
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
            // 3.2) Сигналим ребёнку: файл готов для чтения
            if (semop(semid, &sem_V, 1) == -1) {
                perror("parent semop V");
                break;
            }
        }

        close(fd);
        close(pipefd[0]);
        waitpid(pid, NULL, 0);

        // Удаляем семафор
        if (semctl(semid, 0, IPC_RMID) == -1) {
            perror("semctl IPC_RMID");
        }
        return 0;
    }
}