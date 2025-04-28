#include "contacts_file.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

// При старте читаем все записи
int load_contacts(Contacts_book* book, char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        if (errno == ENOENT) {
            // Файл не существует — просто оставляем книгу пустой
            book->count = 0;
            book->next_id = 1;
            return 0;
        } else {
            perror("Ошибка открытия файла для чтения");
            return -1;
        }
    }
    // Заполняем данные книги
    book->count = 0;
    book->next_id = 1;
    Contact tmp;
    ssize_t r;
    while ((r = read(fd, &tmp, sizeof(tmp))) == sizeof(tmp)) {
        book->contacts[book->count++] = tmp;
        if (tmp.id >= book->next_id)
            book->next_id = tmp.id + 1;
    }
    if (r < 0) {
        perror("Ошибка при чтении файла");
        close(fd);
        return -1;
    }
    // Всё прочитано (r == 0 — EOF)
    close(fd);
    return 0;
}

// При выходе сохраняем всю книгу целиком
int save_contacts(Contacts_book* book, char* filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Ошибка открытия файла для записи");
        return -1;
    }

    // Вносим данные книги в файл
    if (book->count > 0) {
        ssize_t to_write = book->count * (ssize_t)sizeof(Contact);
        ssize_t written = write(fd, book->contacts, to_write);
        if (written != to_write) {
            perror("Ошибка при записи файла");
            close(fd);
            return -1;
        }
    }
    close(fd);
    return 0;
}