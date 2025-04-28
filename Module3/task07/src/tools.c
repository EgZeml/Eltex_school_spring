#include "tools.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void die(char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

mqd_t mq_generate(struct mq_attr *attr, int create) {
    attr->mq_flags   = 0;
    attr->mq_maxmsg  = 10;
    attr->mq_msgsize = 256;
    attr->mq_curmsgs = 0;

    int oflags = O_RDWR;

    if (create) {
        oflags |= O_CREAT | O_EXCL;
    }

    mqd_t mqd = mq_open(QUEUE_NAME, oflags, QUEUE_PERMS, create ? attr : NULL);    

    if (mqd == (mqd_t)-1)
    die("mq_open");

    return mqd;
}

void remove_queue(mqd_t mqd) {
    mq_close(mqd);
    mq_unlink(QUEUE_NAME);
}

void start_chat(mqd_t mqd, struct mq_attr *attr, int  my_prio, int  peer_prio, const char *label) {

    char buf[attr->mq_msgsize];
    int prio;

    while(1) {
        // 1) ===== Отправка нашего сообщения =====
        printf("%s> ", label);
        fflush(stdout);

        // 1.1) Считываем наше сообщение
        if (!fgets(buf, sizeof(buf), stdin)) {
            die("fgets");
        }

        // 1.2) Проверяем сообщения на наличие выхода и присваиваем приоритет
        if (strncmp(buf, STOP_WORD, strlen(STOP_WORD)) == 0) {
            prio = STOP_TYPE;
        } else {
            prio = my_prio;
        }

        // 1.3) Пытаемся отправить сообщение
        if (mq_send(mqd, buf, strlen(buf)+1, prio) == -1) {
            die("mq_send");
        }

        // 1.4) Если сообщение содержало выход, то выходим
        if (prio == STOP_TYPE) {
            printf("Чат завершён\n");
            break;
        }

        // 2) ===== Получение сообщения собеседника =====
        // 2.1) Ждем получение сообщения
        unsigned int rprio;
        do {
            if (mq_receive(mqd, buf, sizeof buf, &rprio) == -1)
                die("mq_receive");
        } while (rprio != peer_prio && rprio != STOP_TYPE);

        if (rprio == STOP_TYPE) {
            puts("Собеседник завершил чат.");
            break;
        }

        printf("Собеседник: %s", buf);
    }
    remove_queue(mqd);
}