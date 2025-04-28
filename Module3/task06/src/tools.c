#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "tools.h"

key_t key_generate(char *path) {
    key_t key = ftok(path, 'A');
    if (key == -1) {
        perror("ftok");
    }
    return key;
}

// Сбрасываем все оставшиеся сообщения, но не шумим, если очередь уже удалена
void flush_msgs(int msqid) {
    struct msgbuf msg;
    ssize_t r;
    while ((r = msgrcv(msqid, &msg, sizeof(msg.mtext), 0, IPC_NOWAIT)) != -1);
    // Если очередь не пуста и не удалена
    if (errno != ENOMSG && errno != EINVAL && errno != EIDRM) {
        perror("msgrcv (flush)");
    }
}

// Удаляем очередь и пытаемся очистить — тоже тихо игнорируем EINVAL/EIDRM
void remove_queue(int msqid) {
    flush_msgs(msqid);
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        // Если очередь не удалена
        if (errno != EINVAL && errno != EIDRM) {
            perror("msgctl(IPC_RMID)");
        }
    }
}

// Проверяем наличие сообщения о выходе в очереди
void exit_check(int msqid, ssize_t r) {
    if (r > 0) {
        printf("Собеседник завершил чат.\n");
        remove_queue(msqid);
        exit(EXIT_SUCCESS);
    } else if (r == -1 && errno != ENOMSG) {
        if (errno == EINVAL || errno == EIDRM) {
            // Очередь уже удалена — считаем, что собеседник вышел
            printf("Собеседник завершил чат.\n");
            exit(EXIT_SUCCESS);
        } else {
            perror("msgrcv (STOP_TYPE)");
            remove_queue(msqid);
            exit(EXIT_FAILURE);
        }
    }
}

void start_chat(int msqid, long send_type, long recv_type, const char *label) {
    struct msgbuf msg;

    while (1) {
        // 1) Сначала проверяем, не прислал ли собеседник STOP_TYPE / не удалина ли очередь
        ssize_t r = msgrcv(msqid, &msg, sizeof(msg.mtext), STOP_TYPE, IPC_NOWAIT);
        exit_check(msqid, r);

        // 2) Вводим и отправляем своё сообщение
        printf("%s: ", label);
        if (!fgets(msg.mtext, sizeof(msg.mtext), stdin)) {
            perror("fgets");
            break;
        }

        // 2.1) Если ввели слово для выхода — шлём STOP_TYPE и уходим
        if (strncmp(msg.mtext, STOP_WORD, strlen(STOP_WORD)) == 0) {
            msg.mtype = STOP_TYPE;
            if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
                perror("msgsnd (STOP_TYPE)");
            }
            printf("Вы завершили чат.\n");
            remove_queue(msqid);
            exit(EXIT_SUCCESS);
            }

        // 2.2) Обычное сообщение
        msg.mtype = send_type;
        if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
            if (errno == EINVAL || errno == EIDRM) {
                printf("Собеседник завершил чат.\n");
                remove_queue(msqid);
                exit(EXIT_SUCCESS);
            } else {
                perror("msgsnd");
                remove_queue(msqid);
                exit(EXIT_FAILURE);
            }
        }

        // 3) Снова проверяем, не прислал ли собеседник STOP_TYPE / не удалил ли очередь
        r = msgrcv(msqid, &msg, sizeof(msg.mtext), STOP_TYPE, IPC_NOWAIT);
        exit_check(msqid, r);

        // 4) Ждём ответ от Собеседника
        // Если не удалось считать, то обрабатывает ошибки 
        if (msgrcv(msqid, &msg, sizeof(msg.mtext), recv_type, 0) == -1) {
            if (errno == EINVAL || errno == EIDRM) {
                printf("Собеседник завершил чат.\n");
                remove_queue(msqid);
                exit(0);
            } else {
                perror("msgrcv (recv_type)");
                remove_queue(msqid);
                exit(1);
            }
        }

        // 5) Выводим сообщение от собеседника
        printf("Собеседник: %s", msg.mtext);
    }

    // Если выходим из цикла по неожиданной ошибке
    remove_queue(msqid);
    exit(1);
}

