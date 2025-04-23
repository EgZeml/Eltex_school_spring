#ifndef CONTACTS_TYPES_H
#define CONTACTS_TYPES_H

#define INFO_LEN_MAX 50 // Количество символов в строке с информацией
#define CONTACTS_LEN_MAX 100 // Количество контактов в книге
#define SOCIALS_LEN_MAX 10 // Количество социальных сетей у контакта

typedef struct {
    char first_name[INFO_LEN_MAX];
    char last_name[INFO_LEN_MAX];
} Name;

typedef struct {
    char workplace[INFO_LEN_MAX];
    char job[INFO_LEN_MAX];
} Work;

typedef struct {
    char operator[INFO_LEN_MAX];
    char number[INFO_LEN_MAX];
} Phone;

typedef struct {
    char social_name[INFO_LEN_MAX];
    char social_link[INFO_LEN_MAX];
} Social;

typedef struct {
    int id;
    Name name;
    Work work;
    Phone phone;
    Social socials[SOCIALS_LEN_MAX];
    char mail[INFO_LEN_MAX];
} Contact;

typedef struct {
    Contact contacts[CONTACTS_LEN_MAX];
    int count;
    int next_id;
} Contacts_book;

#endif