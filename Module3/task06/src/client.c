#include "tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>

int main() {
    // Генерируем ключ
    key_t key = key_generate("./src/server.c");
    if (key == -1) {
        exit(EXIT_FAILURE);
    }

    int msqid = msgget(key, IPC_CREAT | QUEUE_PERMS);
    if (msqid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    // Ждём первого “пинга” от сервера ---
    struct msgbuf msg;
    ssize_t    r = msgrcv(msqid, &msg, sizeof(msg.mtext), 1, 0);
    if (r == -1) {
        perror("msgrcv initial");
        exit(EXIT_FAILURE);
    }
    // выведем его на экран
    printf("Сервер: %s", msg.mtext);
    
    // 1 = сервер, 2 = клиент
    start_chat(msqid, 2, 1, "Клиент");
    return 0;
}
