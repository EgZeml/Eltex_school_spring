#include "tools.h"

int main(void) {

    mq_unlink(QUEUE_NAME);

    struct mq_attr attr;
    // Как сервер - создаём очередь
    mqd_t mqd = mq_generate(&attr, 1);

    start_chat(mqd, &attr, 1, 2, "Сервер");
    return 0;
}
