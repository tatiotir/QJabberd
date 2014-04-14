#ifndef CONTACT_H
#define CONTACT_H

#include <iostream>
#include <ostream>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include <QStringList>
#include <QVariant>

using namespace std;

class Contact
{
public:
    Contact();
    Contact(QString version, bool approved, QString ask, QString jid, QString name,
            QString subscription, QSet<QString> groupList);

    QString getVersion();
    bool getApproved();
    QString getAsk();
    QString getJid();
    QString getName();
    QString getSubscription();
    QSet<QString> getGroups();
    QJsonObject toJsonObject();
    static Contact fromJsonObject(QJsonObject jsonObject);

    void setVersion(QString version);
    void setApproved(bool approved);
    void setAsk(QString ask);
    void setJid(QString jid);
    void setName(QString name);
    void setSubscription(QString subscription);
    void addGroup(QString group);
    void setGroups(QSet<QString> groups);
    void display(ostream& flux);

private:
    QString m_version;
    bool m_approved;
    QString m_ask;
    QString m_jid;
    QString m_name;
    QString m_subscription;
    QSet<QString> m_groupList;
};

ostream& operator<<(std::ostream& flux, Contact contact);

#endif // CONTACT_H
