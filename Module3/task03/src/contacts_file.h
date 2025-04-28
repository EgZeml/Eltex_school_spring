#ifndef CONTACTS_FILE_H
#define CONTACTS_FILE_H

#include "contacts_types.h"

// Загружает все контакты из файла filename в book.
// Если файла нет — оставляет book пустым.
// Возвращает 0 в любом случае, или –1 при серьёзной ошибке.
int load_contacts(Contacts_book* book, char* filename);

// Сохраняет все контакты из book в файл filename,
// перезаписывая его целиком.
// Возвращает 0 при успехе, –1 при ошибке.
int save_contacts(Contacts_book* book, char* filename);

#endif // CONTACTS_FILE_H
