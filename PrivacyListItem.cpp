#include "PrivacyListItem.h"

PrivacyListItem::PrivacyListItem()
{

}

PrivacyListItem::PrivacyListItem(QString type, QString value, QString action, int order,
                                 QSet<QString> itemChildElements)
{
    m_type = type;
    m_value = value;
    m_action = action;
    m_order = order;
    m_itemChildElements = itemChildElements;
}

PrivacyListItem PrivacyListItem::fromJsonObject(QJsonObject object)
{
    return PrivacyListItem(object.value("type").toString(),
                    object.value("value").toString(),
                    object.value("action").toString(),
                    object.value("order").toInt(),
                    object.value("childs").toVariant().toStringList().toSet());
}

QJsonObject PrivacyListItem::toJsonObject()
{
    QJsonObject jsonObject;

    jsonObject.insert("type", m_type);
    jsonObject.insert("value", m_value);
    jsonObject.insert("action", m_action);
    jsonObject.insert("order", m_order);
    jsonObject.insert("childs", QJsonArray::fromStringList(QStringList::fromSet(m_itemChildElements)));

    return jsonObject;
}

QDomElement PrivacyListItem::toXmlElement()
{
    QDomDocument document;
    QDomElement item = document.createElement("item");

    item.setAttribute("type", m_type);
    item.setAttribute("value", m_value);
    item.setAttribute("action", m_action);
    item.setAttribute("order", m_order);

    for (int i = 0, c = m_itemChildElements.count(); i < c; ++i)
    {
        item.appendChild(document.createElement(m_itemChildElements.values().value(i)));
    }
    return item;
}

void PrivacyListItem::setItemChildElements(QSet<QString> itemChildElements)
{
    m_itemChildElements = itemChildElements;
}

QSet<QString> PrivacyListItem::getItemChildElements()
{
    return m_itemChildElements;
}

int PrivacyListItem::getOrder()
{
    return m_order;
}

void PrivacyListItem::setOrder(int order)
{
    m_order = order;
}

QString PrivacyListItem::getValue()
{
    return m_value;
}

void PrivacyListItem::setValue(QString value)
{
    m_value = value;
}

QString PrivacyListItem::getType()
{
    return m_type;
}

void PrivacyListItem::setType(QString type)
{
    m_type = type;
}

QString PrivacyListItem::getAction()
{
    return m_action;
}

void PrivacyListItem::setAction(QString action)
{
    m_action = action;
}

QSet<QString> PrivacyListItem::getChildsElement()
{
    return m_itemChildElements;
}

void PrivacyListItem::setChildsElement(QSet<QString> childs)
{
    m_itemChildElements = childs;
}

bool operator <(PrivacyListItem item1, PrivacyListItem item2)
{
    return (item1.getOrder() < item2.getOrder() ? true : false);
}
