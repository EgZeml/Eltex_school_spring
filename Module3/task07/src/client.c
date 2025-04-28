#include "tools.h"

int main(void){
    struct mq_attr attr;
    // Как клиент - просто открываем существующую очередь
    mqd_t mqd = mq_generate(&attr, 0);

    // Ждём «пинга» от сервера
    unsigned int rprio;
    char buf[attr.mq_msgsize];
    if (mq_receive(mqd, buf, sizeof buf, &rprio) == -1) {
        die("mq_receive");
    }
    printf("Собеседник: %s", buf);

    start_chat(mqd, &attr, 2, 1, "Клиент");
    return 0;
}