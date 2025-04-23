#include "contacts_ui.h"
#include "input_utils.h"

// Сбор данных для добавления контакта в массив
Contact add_contact_ui() {
    Contact new_contact = {0};
    char answer_tmp;
    int number_tmp;

    // Обязательный ввод имени и фамилии
    do {
        printf("Введите имя: ");
        input_string(new_contact.name.first_name, INFO_LEN_MAX);

        printf("Введите фамилию: ");
        input_string(new_contact.name.last_name, INFO_LEN_MAX);
    } while (new_contact.name.first_name[0] == '\0' | new_contact.name.last_name[0] == '\0');

    printf("Вписать информацию о работе? y/n\n");
    scanf(" %c", &answer_tmp);
    flush_input();
    if (answer_tmp == 'y' || answer_tmp == 'Y') {
        printf("Введите место работы: ");
        input_string(new_contact.work.workplace, INFO_LEN_MAX);

        printf("Введите должность: ");
        input_string(new_contact.work.job, INFO_LEN_MAX);
    }

    printf("Вписать информацию о телефоне? y/n\n");
    scanf(" %c", &answer_tmp);
    flush_input();
    if (answer_tmp == 'y' || answer_tmp == 'Y') {
        printf("Введите имя оператора: ");
        input_string(new_contact.phone.operator, INFO_LEN_MAX);

        printf("Введите номер: ");
        input_string(new_contact.phone.number, INFO_LEN_MAX);
    }

    printf("Вписать информацию о социальных сетях? y/n\n");
    scanf(" %c", &answer_tmp);
    flush_input();
    if (answer_tmp == 'y' || answer_tmp == 'Y') {
        printf("Введите количество записей: ");
        scanf(" %d", &number_tmp);
        flush_input();
        for (int i = 0; i < number_tmp; i++) {
            printf("Введите название соцсети: ");
            input_string(new_contact.socials[i].social_name, INFO_LEN_MAX);
    
            printf("Введите ссылки или никнейм: ");
            input_string(new_contact.socials[i].social_link, INFO_LEN_MAX);
        }
    }

    printf("Вписать информацию о почте? y/n\n");
    scanf(" %c", &answer_tmp);
    flush_input();
    if (answer_tmp == 'y' || answer_tmp == 'Y') {
        printf("Введите адрес почты: ");
        input_string(new_contact.mail, INFO_LEN_MAX);
    }

    return new_contact;
}

// Создание данных для добавления тестового контакта в массив
Contact add_test_contact_ui() {
    Contact new_contact = {0};

    strcpy(new_contact.name.first_name, "Egor");
    strcpy(new_contact.name.last_name, "Zemlyanoi");
    strcpy(new_contact.work.workplace, "NSTU");
    strcpy(new_contact.work.job, "Student");
    strcpy(new_contact.phone.operator, "Yota");
    strcpy(new_contact.phone.number, "987-654-32-10");
    strcpy(new_contact.socials[0].social_name, "VK");
    strcpy(new_contact.socials[0].social_link, "EgZeml");
    strcpy(new_contact.socials[1].social_name, "TG");
    strcpy(new_contact.socials[1].social_link, "EgZeml");
    strcpy(new_contact.socials[2].social_name, "INST");
    strcpy(new_contact.socials[2].social_link, "EgZeml");
    strcpy(new_contact.mail, "EgZeml@mail");
    return new_contact;
}

// Сбор данных для изменения контакта в массиве
Contact edit_contact_ui(Contacts_book* book, int index) {
    Contact updated = {0};
    Contact c = book->contacts[index];
    char answer;

    printf("Редактировать имя? y/n: ");
    scanf(" %c", &answer);
    flush_input();
    if (answer == 'y' || answer == 'Y') {
        printf("Введите новое имя: ");
        input_string(updated.name.first_name, INFO_LEN_MAX);
    }

    printf("Редактировать фамилию? (y/n): ");
    scanf(" %c", &answer);
    flush_input();
    if (answer == 'y' || answer == 'Y') {
        printf("Введите новую фамилию: ");
        input_string(updated.name.last_name, INFO_LEN_MAX);
    }

    printf("Редактировать информацию о работе? (y/n): ");
    scanf(" %c", &answer);
    flush_input();
    if (answer == 'y' || answer == 'Y') {
        printf("Введите место работы: ");
        input_string(updated.work.workplace, INFO_LEN_MAX);

        printf("Введите должность: ");
        input_string(updated.work.job, INFO_LEN_MAX);
    }

    printf("Редактировать информацию о телефоне? (y/n): ");
    scanf(" %c", &answer);
    flush_input();
    if (answer == 'y' || answer == 'Y') {
        printf("Введите имя оператора: ");
        input_string(updated.phone.operator, INFO_LEN_MAX);

        printf("Введите номер телефона: ");
        input_string(updated.phone.number, INFO_LEN_MAX);
    }

    printf("Редактировать информацию о социальных сетях? (y/n): ");
    scanf(" %c", &answer);
    flush_input();
    if (answer == 'y' || answer == 'Y') {
        // Проходим каждую запись о социальной сети
        for (int i = 0; i < SOCIALS_LEN_MAX; i++) {
            if (c.socials[i].social_name[0] != '\0') {
                printf("Изменить эту запись: \"%s %s\"? y/n\n", c.socials[i].social_name, c.socials[i].social_link);
                scanf(" %c", &answer);
                flush_input();
                if (answer == 'y' || answer == 'Y') {
                    printf("Введите название соцсети: ");
                    input_string(updated.socials[i].social_name, INFO_LEN_MAX);
            
                    printf("Введите ссылку или никнейм: ");
                    input_string(updated.socials[i].social_link, INFO_LEN_MAX);
                }    
            }
        }
    }

    printf("Редактировать информацию о почте? (y/n): ");
    scanf(" %c", &answer);
    flush_input();
    if (answer == 'y' || answer == 'Y') {
        printf("Введите новый email: ");
        input_string(updated.mail, INFO_LEN_MAX);
    }

    return updated;
}

// Вывод контакта
void display_contact( Contact* c) {
    if (c->name.first_name[0] == '\0') {
        return;
    }
    printf("ID: %d\n", c->id);
    printf("Имя: %s %s\n", c->name.first_name, c->name.last_name);
    printf("Работа: %s %s\n", c->work.workplace, c->work.job);
    printf("Телефон: %s %s\n", c->phone.operator, c->phone.number);
    for (int i = 0; i < SOCIALS_LEN_MAX; i++) {
        if (c->socials[i].social_name[0] != '\0') {
            printf("Соцсети: %s %s\n", c->socials[i].social_name, c->socials[i].social_link);
        }    
    }
    printf("Почта: %s\n", c->mail);
}

// Вывод всех контактов
void display_all_contacts(Contacts_book* book) {
    for (int i = 0; i < book->count; i++) {
        display_contact(&book->contacts[i]);
        printf("\n");
    }
}