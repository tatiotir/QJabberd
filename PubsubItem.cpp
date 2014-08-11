#include "PubsubItem.h"

PubsubItem::PubsubItem(QString id, QByteArray data)
{
    m_id = id;
    m_data = data;
}

PubsubItem::PubsubItem()
{
    m_id = "";
    m_data = "";
}

QString PubsubItem::id() const
{
    return m_id;
}

void PubsubItem::setId(const QString &id)
{
    m_id = id;
}
QByteArray PubsubItem::data() const
{
    return m_data;
}

void PubsubItem::setData(const QByteArray &data)
{
    m_data = data;
}

QJsonObject PubsubItem::toJsonObject()
{
    QJsonObject object;
    object.insert("id", m_id);
    object.insert("data", QString(m_data));
    return object;
}

PubsubItem PubsubItem::fromJsonObject(QJsonObject object)
{
    return PubsubItem(object.value("id").toString(), object.value("data").toString().toUtf8());
}
