#include "Contact.h"

/*!
 * Construct a new contact
 *
 * \brief Contact::Contact
 * \param version
 * \param approved
 * \param ask
 * \param jid
 * \param name
 * \param subscription
 * \param groupList
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

/*!
 * \brief Contact::Contact Default constructor for a contact
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

/*!
 * \brief The Contact::getVersion method return the version of the contact.
 * @return QString
 */
QString Contact::getVersion()
{
    return m_version;
}

/*!
 * \brief The Contact::getApproved method return the approved attribute of the contact
 * @return bool
 */
bool Contact::getApproved()
{
    return m_approved;
}

/*!
 * \brief The Contact::getAsk method return the ask attribute of the contact
 * @return QString
 */
QString Contact::getAsk()
{
    return m_ask;
}

/*!
 * \brief The Contact::getJid method return the jid of the contact
 * @return QString
 */
QString Contact::getJid()
{
    return m_jid;
}

/*!
 * \brief The Contact::getName method return the name of the contact
 * @return QString
 */
QString Contact::getName()
{
    return m_name;
}

/*!
 * \brief The Contact::getSubscription method return the subscription attribute of the contact
 * @return QString
 */
QString Contact::getSubscription()
{
    return m_subscription;
}

/*!
 * \brief The Contact::setVersion method set the version attribute of the contact
 * \param version
 */
void Contact::setVersion(QString version)
{
    m_version = version;
}

/*!
 * \brief The Contact::setApproved method set the approved attribute of the contact
 * \param approved
 */
void Contact::setApproved(bool approved)
{
    m_approved = approved;
}

/*!
 * \brief The Contact::setAsk method set the ask attribute of the contact
 * \param ask
 */
void Contact::setAsk(QString ask)
{
    m_ask = ask;
}

/*!
 * \brief The Contact::setJid method set the jid of the contact
 * \param jid
 */
void Contact::setJid(QString jid)
{
    m_jid = jid;
}

/*!
 * \brief The Contact::setName method set the name of the contact
 * \param name
 */
void Contact::setName(QString name)
{
    m_name = name;
}

/*!
 * \brief The Contact::setSubscription method set the subscription of the contact
 * \param subscription
 */
void Contact::setSubscription(QString subscription)
{
    m_subscription = subscription;
}

/*!
 * \brief The Contact::setGroups method set the groups of the contact
 * \param groups
 */
void Contact::setGroups(QSet<QString> groups)
{
    m_groupList = groups;
}

/*!
 * \brief The Contact::addGroup method add group to group list of the contact
 * \param group
 */
void Contact::addGroup(QString group)
{
    m_groupList << group;
}

/*!
 * \brief The Contact::getGroups method return the group list of the contact
 * @return QSet<String>
 */
QSet<QString> Contact::getGroups()
{
    return m_groupList;
}

/*!
 * \brief The Contact::toJsonObject method return the json representation of this Contact
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

/*!
 * \brief The Contact::fromJsonObject method return the Contact object from a json object
 * \param jsonObject
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

/*!
 * \brief The Contact::display method display a contact in a stream
 * \param flux
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
