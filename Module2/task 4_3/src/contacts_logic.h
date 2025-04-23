#ifndef CONTACTS_LOGIC_H
#define CONTACTS_LOGIC_H

#include "contacts_types.h"

int compare_contacts(Contact*, Contact*);
Tree_node* insert(Tree_node*, Contact*);
Tree_node* search (Tree_node*, int);
Tree_node* find_min(Tree_node*);
Tree_node* delete_node(Tree_node*, int);
int add_logic(Contacts_book* book, Contact* new_contact);
int delete_logic(Contacts_book* book, int id);
int edit_logic(Contacts_book* book, int id, Contact* updated_contact);

#endif