#include "contacts_logic.h"

// Поиск индекса контакта в массиве по id
int find_contact_index(Contacts_book* book, int id) {
    for (int i = 0; i < book->count; i++) {
        if (book->contacts[i].id == id) {
            return i;
        }
    }
    return -1;
}

// Логика добавления контакта в массив
int add_contact_logic(Contacts_book* book, Contact* new_contact) {
    if (book->count >= CONTACTS_LEN_MAX) {
        return -1;
    }
    Contact contact = *new_contact;
    contact.id = book->next_id++;
    book->contacts[book->count++] = contact;
    return 0;
}

// Логика изменения данных контакта
int edit_contact_logic(Contacts_book* book, int index, Contact* updated_contact) {
    Contact* c = &book->contacts[index];
    if (updated_contact->name.first_name[0] != '\0')
        strcpy(c->name.first_name, updated_contact->name.first_name);

    if (updated_contact->name.last_name[0] != '\0')
        strcpy(c->name.last_name, updated_contact->name.last_name);

    if (updated_contact->work.job[0] != '\0')
        strcpy(c->work.job, updated_contact->work.job);

    if (updated_contact->work.workplace[0] != '\0')
        strcpy(c->work.workplace, updated_contact->work.workplace);

    if (updated_contact->phone.operator[0] != '\0')
        strcpy(c->phone.operator, updated_contact->phone.operator);

    if (updated_contact->phone.number[0] != '\0')
        strcpy(c->phone.number, updated_contact->phone.number);

    for (int i = 0; i < SOCIALS_LEN_MAX; i++) {
        if (updated_contact->socials[i].social_name[0] != '\0') {
            strcpy(c->socials[i].social_name, updated_contact->socials[i].social_name);
            strcpy(c->socials[i].social_link, updated_contact->socials[i].social_link);
        }
    }

    if (updated_contact->mail[0] != '\0')
        strcpy(c->mail, updated_contact->mail);

    return 0;
}

// Логика удаления (зануления) контакта
int delete_contact_logic(Contacts_book* book, int id) {
    int index = find_contact_index(book, id);
    if (index == -1) {
        return -1;
    }
    Contact* c = &book->contacts[index];
    c->name.first_name[0] = '\0';
    c->name.last_name[0] = '\0';
    c->work.job[0] = '\0';
    c->work.workplace[0] = '\0';
    c->phone.operator[0] = '\0';
    c->phone.number[0] = '\0';
    for (int i = 0; i < SOCIALS_LEN_MAX; i++) {
        c->socials[i].social_name[0] = '\0';
        c->socials[i].social_link[0] = '\0';
    }
    c->mail[0] = '\0';
    return 0;
}