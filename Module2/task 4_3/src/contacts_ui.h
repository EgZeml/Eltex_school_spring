#ifndef CONTACTS_UI_H
#define CONTACTS_UI_H

#include "contacts_types.h"

Contact add_contact_ui();
Contact add_test_contact_ui();
Contact edit_contact_ui(Tree_node*);
void display_contact(Contact*);
void inorder(Tree_node* root);

#endif