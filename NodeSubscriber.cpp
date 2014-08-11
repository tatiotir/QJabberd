#include "NodeSubscriber.h"

NodeSubscriber::NodeSubscriber(QString jid, QString affiliation, QString subscription)
{
    m_jid = jid;
    m_affiliation = affiliation;
    m_subscription = subscription;
}

NodeSubscriber::NodeSubscriber()
{
    m_jid = "";
    m_affiliation = "";
    m_subscription = "";
}

QString NodeSubscriber::jid() const
{
    return m_jid;
}

void NodeSubscriber::setJid(const QString &jid)
{
    m_jid = jid;
}
QString NodeSubscriber::affiliation() const
{
    return m_affiliation;
}

void NodeSubscriber::setAffiliation(const QString &affiliation)
{
    m_affiliation = affiliation;
}
QString NodeSubscriber::subscription() const
{
    return m_subscription;
}

void NodeSubscriber::setSubscription(const QString &subscription)
{
    m_subscription = subscription;
}

QJsonObject NodeSubscriber::toJsonObject()
{
    QJsonObject object;
    object.insert("jid", m_jid);
    object.insert("affiliation", m_affiliation);
    object.insert("subscription", m_subscription);
    return object;
}

NodeSubscriber NodeSubscriber::fromJsonObject(QJsonObject object)
{
    return NodeSubscriber(object.value("jid").toString(), object.value("affiliation").toString(),
                          object.value("subscription").toString());
}
