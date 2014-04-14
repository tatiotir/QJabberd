#ifndef PRIVACYLISTITEM_H
#define PRIVACYLISTITEM_H

#include <iostream>
#include <QSet>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStringList>
#include <QDomElement>

class PrivacyListItem
{
public:
    PrivacyListItem();
    PrivacyListItem(QString type, QString value, QString action, int order, QSet<QString> itemChildElements);

    void setItemChildElements(QSet<QString> itemChildElements);
    QSet<QString> getItemChildElements();
    int getOrder();
    void setOrder(int order);
    QJsonObject toJsonObject();
    QDomElement toXmlElement();
    bool isLessThan(PrivacyListItem &item);

private:
    QString m_type;
    QString m_value;
    QString m_action;
    int m_order;
    QSet<QString> m_itemChildElements;
};

bool operator <(PrivacyListItem item1, PrivacyListItem item2);

#endif // PRIVACYLISTITEM_H
