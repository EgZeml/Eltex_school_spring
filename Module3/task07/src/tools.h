#ifndef TOOLS_H
#define TOOLS_H

#include <mqueue.h>
#include <sys/stat.h>

/* Очередь одна и та же для обоих процессов */
#define QUEUE_NAME   "/myqueue"
#define QUEUE_PERMS  (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)

#define STOP_WORD "exit\n"   /* что вводим с клавиатуры, чтобы выйти */
#define STOP_TYPE 99         /* приоритет «стоп-кадра» */

mqd_t mq_generate(struct mq_attr *attr, int create);
void  remove_queue(mqd_t mqd);
void  start_chat(mqd_t mqd,
                 struct mq_attr *attr,
                 int  my_prio,
                 int  peer_prio,
                 const char   *label);

#endif /* TOOLS_H */
