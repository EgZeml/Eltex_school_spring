#ifndef TOOLS_H
#define TOOLS_H

#include <sys/ipc.h>
#include <sys/msg.h>

#define QUEUE_PERMS 0666      // Права очереди сообщений
#define STOP_TYPE   99        // Тип сообщения для выхода
#define STOP_WORD   "exit"    // Слово для завершения чата

struct msgbuf {
    long mtype;
    char mtext[256];
};

/**
 * Генерирует ключ на основе переданного пути
 * @param path — путь к файлу для ftok
 * @return ключ или -1 при ошибке
 */
key_t key_generate(char *path);

/**
 * Запускает цикл персонального чата
 * @param msqid — идентификатор очереди сообщений
 * @param send_type — mtype для своих сообщений (1 или 2)
 * @param recv_type — mtype для входящих сообщений
 * @param label — префикс в приглашении (например, "Сервер" или "Клиент")
 */
void start_chat(int msqid, long send_type, long recv_type, const char *label);

/**
 * Читает все сообщения очереди для очистки
 * @param msqid — идентификатор очереди сообщений
 */
void flush_msgs(int msqid);

/**
 * Выходит и удаляет очередь, если получено сообщение о выходе
 * @param msqid - идентификатор очереди сообщений
 * @param r - размер прочитанного сообщения
 */
void exit_check(int msqid, ssize_t r);


#endif
