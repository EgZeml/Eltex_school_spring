#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <time.h>

#define MAX_READERS 5

// Для управления двумя семафорами в одном наборе
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

/*
=== Описание программы ===
У нас есть читатели, писатель и генератор
- Читатели (MAX_READERS дочерних процессов): читают число из файла
- Писатель (родительский процесс): читает число из потока и записывает число в файл
- Генератор (дочерний процесс): генерирует число и отправляет в поток
Для синхронизации у нас есть 3 семафора:
- sem_rw: Следит за работой с файлом
- sem_data: Следит за готовностью данных
- sem_bARRIER: Следит, что читатели корректно прочитали перед новой итерацией
*/
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <количество чисел>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int amount = atoi(argv[1]);
    if (amount <= 0) {
        fprintf(stderr, "Количество должно быть > 0\n");
        return EXIT_FAILURE;
    }

    // 1) Pipe: генератор -> писатель
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    // 2) Создаем набор из двух семафоров
    // semid[0] = sem_rw, semid[1] = sem_data
    key_t key = ftok(".", 'R');
    if (key == -1) {
        perror("ftok");
        return EXIT_FAILURE;
    }
    int semid = semget(key, 3, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return EXIT_FAILURE;
    }
    union semun arg;
    // sem_rw = MAX_READERS (по умолчанию пустые слоты для читателей)
    arg.val = MAX_READERS;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL sem_rw");
        return EXIT_FAILURE;
    }
    // sem_data = 0 (ни одни данные ещё не готовы)
    arg.val = 0;
    if (semctl(semid, 1, SETVAL, arg) == -1) {
        perror("semctl SETVAL sem_data");
        return EXIT_FAILURE;
    }
    arg.val = 0;
    if (semctl(semid, 2, SETVAL, arg) == -1) {
        perror("semctl SETVAL sem_bARRIER");
        return EXIT_FAILURE;
    }

    // Подготовим операции
    // Cемафор 0: читатель - писатель
    struct sembuf P_reader   = { 0, -1, 0 };
    struct sembuf V_reader   = { 0, +1, 0 };
    struct sembuf P_writer   = { 0, -MAX_READERS, 0 };
    struct sembuf V_writer   = { 0, +MAX_READERS, 0 };
    // Семафор 1: готовность данных
    struct sembuf P_data     = { 1, -1, 0 };
    struct sembuf V_data     = { 1, +1, 0 };
    // Семафор 2: читатели прочитали
    struct sembuf V_barrier  = {2, +1, 0};      // reader ⇒ writer: я прочитал
    struct sembuf P_barrier  = {2, -MAX_READERS,0}; // writer ждёт всех чтений


    // 3) Форкаем MAX_READERS читателей
    for (int i = 0; i < MAX_READERS; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork reader");
            return EXIT_FAILURE;
        }
        if (pid == 0) {
            // Дочерний процесс (читатель)
            close(pipefd[0]);
            close(pipefd[1]);
            for (int j = 0; j < amount; ++j) {
                // 1) Ждем сигнала «данные готовы»
                if (semop(semid, &P_data, 1) == -1) {
                    perror("reader P_data");
                    exit(EXIT_FAILURE);
                }
                // 2) Входим в чтение (P_reader)
                if (semop(semid, &P_reader, 1) == -1) {
                    perror("reader P_reader");
                    exit(EXIT_FAILURE);
                }

                // 3) Читаем j-ый элемент из файла
                int fd = open("output.dat", O_RDONLY);
                if (fd < 0) {
                    perror("reader open");
                    semop(semid, &V_reader, 1);
                    exit(EXIT_FAILURE);
                }
                int val;
                ssize_t r = pread(fd, &val, sizeof val, j * sizeof val);
                if (r == sizeof val) {
                    printf("[Процесс %d] a[%d] = %d\n", i, j, val);
                } else {
                    fprintf(stderr, "[Процесс %d] нет данных на позиции %d\n",
                            i, j);
                }
                close(fd);

                // 4) Выходим из чтения (V_reader)
                if (semop(semid, &V_reader, 1) == -1) {
                    perror("reader V_reader");
                    exit(EXIT_FAILURE);
                }

                // 4) Выходим из чтения (V_reader)
                if (semop(semid, &V_barrier, 1) == -1) {
                    perror("reader V_reader");
                    exit(EXIT_FAILURE);
                }
                
            }
            exit(EXIT_SUCCESS);
        }
        // родитель продолжает форкать
    }

    // 4) Форкаем дочерний процесс (генератор)
    pid_t pid_gen = fork();
    if (pid_gen < 0) {
        perror("fork generator");
        return EXIT_FAILURE;
    }
    if (pid_gen == 0) {
        // --- код генератора чисел ---
        close(pipefd[0]);
        srand(time(NULL) ^ getpid());
        for (int j = 0; j < amount; ++j) {
            int x = rand();
            if (write(pipefd[1], &x, sizeof x) != sizeof x) {
                perror("generator write");
                break;
            }
        }
        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    }

    // 5) Родитель (писатель)
    close(pipefd[1]);
    int fd = open("output.dat", O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0) {
        perror("writer open");
        return EXIT_FAILURE;
    }

    for (int j = 0; j < amount; ++j) {
        // 5.1) Читаем следующее число из pipe
        int x;
        if (read(pipefd[0], &x, sizeof x) != sizeof x) {
            perror("writer read pipe");
            break;
        }

        // 5.2) Захватываем доступ (ждём, пока нет читателей)
        if (semop(semid, &P_writer, 1) == -1) {
            perror("writer P_writer");
            break;
        }

        // 5.3) Записываем в файл
        if (write(fd, &x, sizeof x) != sizeof x) {
            perror("writer write file");
        } else {
            printf("[Writer] a[%d] = %d\n", j, x);
        }
        fsync(fd);

        // 5.4) Освобождаем всех читателей
        if (semop(semid, &V_writer, 1) == -1) {
            perror("writer V_writer");
            break;
        }
        // 5.5) Сигналим data-ready — каждому читателю по 1 «единице»
        for (int r = 0; r < MAX_READERS; ++r) {
            if (semop(semid, &V_data, 1) == -1) {
                perror("writer V_data");
                break;
            }
        }

        // Ждём, пока все N читателей не поставят свой +1 в sem_bARRIER
        semop(semid, &P_barrier, 1);      
    }

    // Закрываем и чистим
    close(fd);
    close(pipefd[0]);
    // ждём всех потомков (генератор + MAX_READERS читателей)
    while (wait(NULL) > 0) { }
    // удаляем семафоры
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
    }

    return EXIT_SUCCESS;
}
