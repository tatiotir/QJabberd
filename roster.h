#ifndef ROSTER_H
#define ROSTER_H

#include <QString>
#include "contact.h"

class Roster
{
public:
    Roster();

    void addContact(Contact *contact);
    void addContacts(QList<Contact *> contacts);
    void deleteContact(Contact *contact);
    void deleteContacts(QList<Contact *> contacts);
    void updateContact(Contact *oldContact, Contact *newContact);

private:
    QList<Contact *> *m_contactList;
};

#endif // ROSTER_H
