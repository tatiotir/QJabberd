#include "roster.h"

Roster::Roster()
{

}

QString Roster::getVersion()
{
    return m_version;
}

QString Roster::getApproved()
{
    return m_approved;
}

QString Roster::getAsk()
{
    return m_ask;
}

QString Roster::getJid()
{
    return m_jid;
}

QString Roster::getName()
{
    return m_name;
}

QString Roster::getSubscription()
{
    return m_subscription;
}

void Roster::getVersion(QString version)
{
    m_version = version;
}

void Roster::getApproved(QString approved)
{
    m_approved = approved;
}

void Roster::getAsk(QString ask)
{
    m_ask = ask;
}

void Roster::getJid(QString jid)
{
    m_jid = jid;
}

void Roster::getName(QString name)
{
    m_name = name;
}

void Roster::getSubscription(QString subscription)
{
    m_subscription = subscription;
}

void Roster::addContact(Contact *contact)
{

}

void Roster::addContacts(QList<Contact *> contacts)
{

}

void Roster::deleteContact(Contact *contact)
{

}

void Roster::deleteContacts(QList<Contact *> contacts)
{

}

void Roster::updateContact(Contact *oldContact, Contact *newContact)
{

}
