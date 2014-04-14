#include "MetaContact.h"

MetaContact::MetaContact(QString jid, QString tag, int order)
{
    m_jid = jid;
    m_tag = tag;
    m_order = order;
}

MetaContact::MetaContact()
{
    m_jid = "";
    m_tag = "";
    m_order = -1;
}

void MetaContact::setJid(QString jid)
{
    m_jid = jid;
}

QString MetaContact::getJid()
{
    return m_jid;
}

void MetaContact::setOrder(int order)
{
    m_order = order;
}

int MetaContact::getOrder()
{
    return m_order;
}

void MetaContact::setTag(QString tag)
{
    m_tag = tag;
}

QString MetaContact::getTag()
{
    return m_tag;
}

MetaContact MetaContact::fromJsonObject(QJsonObject jsonObject)
{
    MetaContact metaContact;
    metaContact.setJid(jsonObject.value("jid").toString());
    metaContact.setOrder(jsonObject.value("order").toVariant().toInt());
    metaContact.setTag(jsonObject.value("tag").toString());

    return metaContact;
}

QJsonObject MetaContact::toJsonObject()
{
    QJsonObject jsonObject;
    jsonObject.insert("jid", m_jid);
    jsonObject.insert("tag", m_tag);
    jsonObject.insert("order", m_order);

    return jsonObject;
}

bool operator <(MetaContact meta1, MetaContact meta2)
{
    return (meta1.getOrder() < meta2.getOrder() ? true : false);
}
