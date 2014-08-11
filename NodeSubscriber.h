#ifndef NODESUBSCRIBER_H
#define NODESUBSCRIBER_H

#include <QString>
#include <QJsonObject>

class NodeSubscriber
{
public:
    NodeSubscriber(QString jid, QString affiliation, QString subscription);
    NodeSubscriber();

    QString jid() const;
    void setJid(const QString &jid);

    QString affiliation() const;
    void setAffiliation(const QString &affiliation);

    QString subscription() const;
    void setSubscription(const QString &subscription);

    QJsonObject toJsonObject();
    static NodeSubscriber fromJsonObject(QJsonObject object);

private:
    QString m_jid;
    QString m_affiliation;
    QString m_subscription;
};

#endif // NODESUBSCRIBER_H
