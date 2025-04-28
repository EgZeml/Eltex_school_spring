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

    // 1 = сервер, 2 = клиент
    start_chat(msqid, 1, 2, "Сервер");
    return 0;
}
