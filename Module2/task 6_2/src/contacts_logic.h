#ifndef CONTACTS_LOGIC_H
#define CONTACTS_LOGIC_H

#include "contacts_types.h"

int add_contact_logic(Contacts_book*, Contact*);
int edit_contact_logic(Contacts_book*, Node*, Contact*);
int delete_contact_logic(Contacts_book*, Node*);
Node* find_contact_index(Contacts_book*, int);
int compare_contacts(Contact*, Contact*);
void sorted_insert(Contacts_book*, Node*);
void detach_node(Contacts_book*, Node*);

#endif