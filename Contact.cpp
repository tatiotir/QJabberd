#include "Contact.h"

/**
 * Construct a new contact
 *
 * @brief Contact::Contact
 * @param version
 * @param approved
 * @param ask
 * @param jid
 * @param name
 * @param subscription
 * @param groupList
 */
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

/**
 * Default constructor for a contact
 *
 * @brief Contact::Contact
 */
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

/**
 * Get the version of the contact.
 *
 * @brief Contact::getVersion
 * @return QString
 */
QString Contact::getVersion()
{
    return m_version;
}

/**
 * Get the approved attribute of the contact
 *
 * @brief Contact::getApproved
 * @return bool
 */
bool Contact::getApproved()
{
    return m_approved;
}

/**
 * Get the ask attribute of the contact
 *
 * @brief Contact::getAsk
 * @return QString
 */
QString Contact::getAsk()
{
    return m_ask;
}

/**
 * Get the jid of the contact
 *
 * @brief Contact::getJid
 * @return QString
 */
QString Contact::getJid()
{
    return m_jid;
}

/**
 * Get the name of the contact
 *
 * @brief Contact::getName
 * @return QString
 */
QString Contact::getName()
{
    return m_name;
}

/**
 * Get the subscription attribute of the contact
 *
 * @brief Contact::getSubscription
 * @return QString
 */
QString Contact::getSubscription()
{
    return m_subscription;
}

/**
 * Set the version attribute of the contact
 *
 * @brief Contact::setVersion
 * @param version
 */
void Contact::setVersion(QString version)
{
    m_version = version;
}

/**
 * Set the approved attribute of the contact
 *
 * @brief Contact::setApproved
 * @param approved
 */
void Contact::setApproved(bool approved)
{
    m_approved = approved;
}

/**
 * Set the ask attribute of the contact
 *
 * @brief Contact::setAsk
 * @param ask
 */
void Contact::setAsk(QString ask)
{
    m_ask = ask;
}

/**
 * Set the jid of the contact
 *
 * @brief Contact::setJid
 * @param jid
 */
void Contact::setJid(QString jid)
{
    m_jid = jid;
}

/**
 * Set the name of the contact
 *
 * @brief Contact::setName
 * @param name
 */
void Contact::setName(QString name)
{
    m_name = name;
}

/**
 * Set the subscription of the contact
 *
 * @brief Contact::setSubscription
 * @param subscription
 */
void Contact::setSubscription(QString subscription)
{
    m_subscription = subscription;
}

/**
 * Set the groups of the contact
 *
 * @brief Contact::setGroups
 * @param groups
 */
void Contact::setGroups(QSet<QString> groups)
{
    m_groupList = groups;
}

/**
 * Add group to group list of the contact
 *
 * @brief Contact::addGroup
 * @param group
 */
void Contact::addGroup(QString group)
{
    m_groupList << group;
}

/**
 * Get the group list of the contact
 *
 * @brief Contact::getGroups
 * @return QSet<String>
 */
QSet<QString> Contact::getGroups()
{
    return m_groupList;
}

/**
 * Return the json representation of this Contact
 *
 * @brief Contact::toJsonObject
 * @return QJsonObject
 */
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

/**
 * Return the Contact representation from a json object
 *
 * @brief Contact::fromJsonObject
 * @param jsonObject
 * @return Contact
 */
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

/**
 * Display a contact in a stream
 *
 * @brief Contact::display
 * @param flux
 */
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
