#include "Contact.h"

Contact::Contact(QString version, bool approved, QString ask, QString jid, QString name, QString subscription, QSet<QString> groupList)
{
    m_version = version;
    m_approved = approved;
    m_ask = ask;
    m_jid = jid;
    m_name = name;
    m_subscription = subscription;
    m_groupList = groupList;
}

Contact::Contact()
{
    m_version = "";
    m_approved = false;
    m_ask = "";
    m_jid = "";
    m_name = "";
    m_subscription = "";
    m_groupList = QSet<QString>();
}

/*Contact::Contact(Contact &contact)
{
    m_version = contact.getVersion();
    m_approved = contact.getApproved();
    m_ask = contact.getAsk();
    m_jid = contact.getJid();
    m_name = contact.getName();
    m_subscription = contact.getSubscription();
    m_groupList = contact.getGroups();
}*/

QString Contact::getVersion()
{
    return m_version;
}

bool Contact::getApproved()
{
    return m_approved;
}

QString Contact::getAsk()
{
    return m_ask;
}

QString Contact::getJid()
{
    return m_jid;
}

QString Contact::getName()
{
    return m_name;
}

QString Contact::getSubscription()
{
    return m_subscription;
}

void Contact::setVersion(QString version)
{
    m_version = version;
}

void Contact::setApproved(bool approved)
{
    m_approved = approved;
}

void Contact::setAsk(QString ask)
{
    m_ask = ask;
}

void Contact::setJid(QString jid)
{
    m_jid = jid;
}

void Contact::setName(QString name)
{
    m_name = name;
}

void Contact::setSubscription(QString subscription)
{
    m_subscription = subscription;
}

void Contact::setGroups(QSet<QString> groups)
{
    m_groupList = groups;
}

void Contact::addGroup(QString group)
{
    m_groupList << group;
}

QSet<QString> Contact::getGroups()
{
    return m_groupList;
}

QJsonObject Contact::toJsonObject()
{
    QJsonObject jsonObject;
    jsonObject.insert("version", m_version);
    jsonObject.insert("approved", m_approved);
    jsonObject.insert("ask", m_ask);
    jsonObject.insert("jid", m_jid);
    jsonObject.insert("name", m_name);
    jsonObject.insert("subscription", m_subscription);
    jsonObject.insert("groups", QJsonArray::fromStringList(QStringList::fromSet(m_groupList)));

    return jsonObject;
}

Contact Contact::fromJsonObject(QJsonObject jsonObject)
{
    Contact contact;
    contact.setApproved(jsonObject.value("approved").toBool());
    contact.setAsk(jsonObject.value("ask").toString());
    contact.setGroups(jsonObject.value("groups").toVariant().toStringList().toSet());
    contact.setJid(jsonObject.value("jid").toString());
    contact.setName(jsonObject.value("name").toString());
    contact.setSubscription(jsonObject.value("subscription").toString());
    contact.setVersion(jsonObject.value("version").toString());

    return contact;
}

void Contact::display(ostream &flux)
{
    flux << "Contact : ( ( Jid : " << m_jid.toStdString() << " ),";
    flux << "( Name : " << m_name.toStdString() << " ),";
    flux << "( Subscription : " << m_subscription.toStdString() << " ),";
    flux << "( Ask : " <<  m_ask.toStdString() << " ),";
}

ostream& operator<<(ostream& flux, Contact contact)
{
    contact.display(flux);
    return flux;
}
