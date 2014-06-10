#ifndef PRIVACYLISTITEM_H
#define PRIVACYLISTITEM_H

#include <iostream>
#include <QSet>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStringList>
#include <QVariant>
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
    QString getValue();
    void setValue(QString value);
    QString getType();
    void setType(QString type);
    QString getAction();
    void setAction(QString action);
    QSet<QString> getChildsElement();
    void setChildsElement(QSet<QString> childs);
    QJsonObject toJsonObject();
    static PrivacyListItem fromJsonObject(QJsonObject object);
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
bool operator ==(PrivacyListItem item1, PrivacyListItem item2);

#endif // PRIVACYLISTITEM_H
