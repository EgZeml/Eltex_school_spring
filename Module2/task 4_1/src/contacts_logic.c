#include "contacts_logic.h"
#include "contacts_types.h"
#include <stdlib.h>
#include <string.h>

// Сравнение контактов по фамилии: 
// Возвращает -1, если a < b, 0 если равны, 1, если a > b.
int compare_contacts(Contact* a, Contact* b) {
    int cmp = strcasecmp(a->name.last_name, b->name.last_name);
    if (cmp == 0) {
        if (a->id < b->id)
            return -1;
        else if (a->id > b->id)
            return 1;
        else
            return 0;
    }
    return cmp;
}

// Сортированная вставка
void sorted_insert(Contacts_book* book, Node* new_node) {
    // Если список пуст, новый узел становится началом и концом.
    if (book->head == NULL) {
        book->head = book->tail = new_node;
        return;
    }

    // Если новый узел должен быть вставлен в начало.
    if (compare_contacts(&new_node->data, &book->head->data) < 0) {
        new_node->next = book->head;
        book->head->prev = new_node;
        book->head = new_node;
        return;
    }

    // Ищем позицию для вставки: от меньшего к большему
    Node* current = book->head;
    while (current->next != NULL && compare_contacts(&new_node->data, &current->next->data) >= 0) {
        current = current->next;
    }
    
    // Вставляем новый узел после current.
    new_node->next = current->next;
    new_node->prev = current;
    current->next = new_node;
    if (new_node->next != NULL) {
        new_node->next->prev = new_node;
    } else {
        // Если новый узел вставлен в конец, обновляем tail.
        book->tail = new_node;
    }
}

// Логика добавления контакта в массив
int add_contact_logic(Contacts_book* book, Contact* new_contact) {
    if (book->count >= CONTACTS_LEN_MAX) {
        return -1;
    }

    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        return -1; // или обработать ошибку выделения памяти
    }

    new_node->data = *new_contact;
    new_node->data.id = book->next_id++;
    new_node->next = NULL;
    new_node->prev = NULL;

    // Вставляем новый узел с сохранением порядка
    sorted_insert(book, new_node);

    book->count++;

    return 0;
}

// Поиск индекса контакта в массиве по id
Node* find_contact_index(Contacts_book* book, int id) {
    Node* ptr = book->head;
    while (ptr != NULL) {
        if (ptr->data.id == id) {
            return ptr;
        }
        ptr = ptr->next;
    }
    return NULL;
}

// Вспомогательная функция для отсоединения узла из списка без его удаления
void detach_node(Contacts_book* book, Node* node) {
    if (node == book->head) {
        book->head = node->next;
        if (book->head != NULL) {
            book->head->prev = NULL;
        } else {
            book->tail = NULL;
        }
    } else if (node == book->tail) {
        book->tail = node->prev;
        if (book->tail != NULL) {
            book->tail->next = NULL;
        }
    } else {
        if (node->prev) {
            node->prev->next = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        }
    }
    node->next = NULL;
    node->prev = NULL;
}

// Логика изменения данных контакта
int edit_contact_logic(Contacts_book* book, Node* node, Contact* updated_contact) {
    if (node == NULL) {
        return -1;
    }

    // Обновляем поля контакта, если новые данные не пусты.
    if (updated_contact->name.first_name[0] != '\0') {
        strcpy(node->data.name.first_name, updated_contact->name.first_name);
    }
    if (updated_contact->name.last_name[0] != '\0') {
        strcpy(node->data.name.last_name, updated_contact->name.last_name);
    }
    if (updated_contact->work.job[0] != '\0') {
        strcpy(node->data.work.job, updated_contact->work.job);
    }
    if (updated_contact->work.workplace[0] != '\0') {
        strcpy(node->data.work.workplace, updated_contact->work.workplace);
    }
    if (updated_contact->phone.operator[0] != '\0') {
        strcpy(node->data.phone.operator, updated_contact->phone.operator);
    }
    if (updated_contact->phone.number[0] != '\0') {
        strcpy(node->data.phone.number, updated_contact->phone.number);
    }
    for (int i = 0; i < SOCIALS_LEN_MAX; i++) {
        if (updated_contact->socials[i].social_name[0] != '\0') {
            strcpy(node->data.socials[i].social_name, updated_contact->socials[i].social_name);
            strcpy(node->data.socials[i].social_link, updated_contact->socials[i].social_link);
        }
    }
    if (updated_contact->mail[0] != '\0') {
        strcpy(node->data.mail, updated_contact->mail);
    }

    // Отсоединяем узел из списка для переупорядочивания
    detach_node(book, node);
    // Повторно вставляем узел в отсортированном порядке
    sorted_insert(book, node);

    return 0;
}

// Логика удаления (освобождение памяти) контакта
int delete_contact_logic(Contacts_book* book, Node* node) {
    if (node == NULL) {
        return -1;
    }
    
    // Отсоединяем узел из списка.
    detach_node(book, node);
    
    free(node);
    book->count--;
    return 0;
}