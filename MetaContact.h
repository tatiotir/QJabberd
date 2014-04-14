#ifndef METACONTACT_H
#define METACONTACT_H

#include <QString>
#include <QVariant>
#include <QJsonObject>

class MetaContact
{
    public:
    MetaContact(QString jid, QString tag, int order);
    MetaContact();

    void setJid(QString jid);
    QString getJid();
    void setTag(QString tag);
    QString getTag();
    void setOrder(int order);
    int getOrder();

    static MetaContact fromJsonObject(QJsonObject jsonObject);
    QJsonObject toJsonObject();

private:
    QString m_jid;
    QString m_tag;
    int m_order;
};

bool operator <(MetaContact meta1, MetaContact meta2);

#endif // METACONTACT_H
