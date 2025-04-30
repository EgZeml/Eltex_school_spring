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
#include <string.h>

#define READERS 5

static const char *SEM_C2P          = "/sem_c2p";          // генератор -> писатель: сигнал о готовности нового числа
static const char *SEM_P2C          = "/sem_p2c";          // писатель -> генератор: разрешение сгенерировать следующее число
static const char *SEM_FILE_READY   = "/sem_file_ready";   // писатель -> читатели: сигнал, что файл обновлён и готов к чтению
static const char *SEM_READERS_DONE = "/sem_readers_done"; // читатели -> писатель: уведомление, что все читатели прочитали новое значение.

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <amount>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int amount  = atoi(argv[1]);
    if (amount <= 0 || READERS <= 0) {
        fprintf(stderr, "Both <amount> and <readers_count> must be > 0\n");
        return EXIT_FAILURE;
    }

    // Канал для передачи числа от генератора писателю
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    // 1) Создание или открытие именнованных семафоров
    // 1.1) генератор -> писатель: сигнал о готовности нового числа
    sem_t *sem_c2p = sem_open(SEM_C2P, O_CREAT | O_EXCL, 0666, 0);
    if (sem_c2p == SEM_FAILED && errno == EEXIST) {
        sem_c2p = sem_open(SEM_C2P, 0);
    }
    if (sem_c2p == SEM_FAILED) {
        perror("sem_open sem_c2p");
        return EXIT_FAILURE;
    }

    // 1.2) писатель -> генератор: разрешение сгенерировать следующее число
    sem_t *sem_p2c = sem_open(SEM_P2C, O_CREAT | O_EXCL, 0666, 0);
    if (sem_p2c == SEM_FAILED && errno == EEXIST) {
        sem_p2c = sem_open(SEM_P2C, 0);
    }
    if (sem_p2c == SEM_FAILED) {
        perror("sem_open sem_p2c");
        sem_close(sem_c2p);
        sem_unlink(SEM_C2P);
        return EXIT_FAILURE;
    }

    // 1.3) писатель -> читатели: сигнал, что файл обновлён и готов к чтению
    sem_t *sem_file_ready = sem_open(SEM_FILE_READY, O_CREAT | O_EXCL, 0666, 0);
    if (sem_file_ready == SEM_FAILED && errno == EEXIST) {
        sem_file_ready = sem_open(SEM_FILE_READY, 0);
    }
    if (sem_file_ready == SEM_FAILED) {
        perror("sem_open sem_file_ready");
        sem_close(sem_c2p); sem_close(sem_p2c);
        sem_unlink(SEM_C2P); sem_unlink(SEM_P2C);
        return EXIT_FAILURE;
    }

    // 1.4) читатели -> писатель: уведомление, что все читатели прочитали новое значение
    sem_t *sem_readers_done = sem_open(SEM_READERS_DONE, O_CREAT | O_EXCL, 0666, 0);
    if (sem_readers_done == SEM_FAILED && errno == EEXIST) {
        sem_readers_done = sem_open(SEM_READERS_DONE, 0);
    }
    if (sem_readers_done == SEM_FAILED) {
        perror("sem_open sem_readers_done");
        sem_close(sem_c2p); sem_close(sem_p2c);
        sem_close(sem_file_ready);
        sem_unlink(SEM_C2P); sem_unlink(SEM_P2C);
        sem_unlink(SEM_FILE_READY);
        return EXIT_FAILURE;
    }

    // 2) Форкаем генератор (дочерний прочесс)
    pid_t gen_pid = fork();
    if (gen_pid < 0) {
        perror("fork generator");
        return EXIT_FAILURE;
    }
    if (gen_pid == 0) {
        close(pipefd[0]);
        srand(time(NULL) ^ getpid());
        for (int i = 0; i < amount; ++i) {
            int val = rand();
            if (write(pipefd[1], &val, sizeof(val)) != sizeof(val)) {
                perror("generator write");
                break;
            }
            printf("Generator [%d] = %d\n", i, val);
            // 2.1) Сообщаем писателю о готовности числа
            sem_post(sem_c2p);
            // 2.2) Ждем разрешение от писателя сгенерировать новое число
            sem_wait(sem_p2c);
        }
        close(pipefd[1]);
        // 2.3) Закрываем семафоры перед выходом
        sem_close(sem_p2c);
        sem_close(sem_file_ready);
        sem_close(sem_readers_done);
        return EXIT_SUCCESS;
    }

    // 3) Форкаем читателей (дочерние процессы)
    for (int j = 0; j < READERS; ++j) {
        pid_t rpid = fork();
        if (rpid < 0) {
            perror("fork reader");
            return EXIT_FAILURE;
        }
        if (rpid == 0) {
            close(pipefd[0]);
            close(pipefd[1]);
            for (int i = 0; i < amount; ++i) {
                // 3.1) Ждем разрешение на работу с файлом от писателя
                sem_wait(sem_file_ready); 
                int fd = open("output.dat", O_RDONLY);
                if (fd < 0) { 
                    perror("reader open"); 
                    break; 
                }
                int tmp;
                off_t offset = i * sizeof(tmp);
                if (pread(fd, &tmp, sizeof(tmp), offset) == sizeof(tmp)) {
                    printf("\tReader %d: file[%d] = %d\n", j, i, tmp);
                } else {
                    perror("reader pread");
                }
                close(fd);
                // 3.2) Сигналим писателю, что читатель закончил работу
                sem_post(sem_readers_done);
            }
            sem_close(sem_c2p);
            sem_close(sem_p2c);
            sem_close(sem_file_ready);
            sem_close(sem_readers_done);
            return EXIT_SUCCESS;
        }
    }

    // 4) Писатель (родительский процесс)
    close(pipefd[1]);
    int fd = open("output.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("parent open");
        return EXIT_FAILURE;
    }
    for (int i = 0; i < amount; ++i) {
        // 4.1) Ждем сигнал о готовности числа
        sem_wait(sem_c2p);
        int num;
        if (read(pipefd[0], &num, sizeof(num)) != sizeof(num)) {
            perror("parent read"); break;
        }
        printf("Parent received[%d] = %d\n", i, num);
        if (write(fd, &num, sizeof(num)) != sizeof(num)) {
            perror("parent write"); break;
        }
        fsync(fd);
        // 4.2) Отправляем читателям сигналы с разрешением работы в файле (чтение)
        for (int j = 0; j < READERS; ++j) {
            sem_post(sem_file_ready);
        }
        // 4.3) Ждем сигнал от читателей о конце работы с файлом
        for (int j = 0; j < READERS; ++j) {
            sem_wait(sem_readers_done);
        }
        sleep(1); // Спим для красивого вывода
        // 4.4) Сигналим генератору разрешение на генерацию нового числа
        sem_post(sem_p2c);
    }
    close(fd);
    close(pipefd[0]);

    // 5) Ждем завершения процессов
    // 5.1) Генератор закончил 
    waitpid(gen_pid, NULL, 0);
    // 5.1) Писатели закончили 
    for (int j = 0; j < READERS; ++j) wait(NULL);

    // Закрытие и удаление
    sem_close(sem_c2p); sem_close(sem_p2c);
    sem_close(sem_file_ready); sem_close(sem_readers_done);
    sem_unlink(SEM_C2P); sem_unlink(SEM_P2C);
    sem_unlink(SEM_FILE_READY); sem_unlink(SEM_READERS_DONE);

    return EXIT_SUCCESS;
}
