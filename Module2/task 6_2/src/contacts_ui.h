#ifndef CONTACTS_UI_H
#define CONTACTS_UI_H

#include "contacts_types.h"

Contact add_contact_ui();
Contact add_test_contact_ui();
Contact edit_contact_ui(Contacts_book*, Node*);
void display_contact(Contact*);
void display_all_contacts(Contacts_book*);

#endif