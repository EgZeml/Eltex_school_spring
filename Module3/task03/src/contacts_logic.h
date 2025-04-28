#ifndef CONTACTS_LOGIC_H
#define CONTACTS_LOGIC_H

#include "contacts_types.h"

int add_contact_logic(Contacts_book* book, Contact* new_contact);
int edit_contact_logic(Contacts_book* book, int id, Contact* updated_contact);
int delete_contact_logic(Contacts_book* book, int id);
int find_contact_index(Contacts_book* book, int id);

#endif