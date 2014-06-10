#ifndef OCCUPANT_H
#define OCCUPANT_H

#include <QString>
#include <QJsonObject>

class Occupant
{
public:
    Occupant();
    Occupant(QString jid, QString mucJid, QString affiliation, QString role, QString status, QString show);

    QString jid() const;
    void setJid(const QString &jid);

    QString affiliation() const;
    void setAffiliation(const QString &affiliation);

    QString role() const;
    void setRole(const QString &role);

    QString mucJid() const;
    void setMucJid(const QString &mucJid);

    QJsonObject toJsonObject();
    static Occupant fromJsonObject(QJsonObject object);

    QString status() const;
    void setStatus(const QString &status);

    QString show() const;
    void setShow(const QString &show);

private:
    QString m_jid;
    QString m_mucJid;
    QString m_affiliation;
    QString m_role;
    QString m_status;
    QString m_show;
};

#endif // OCCUPANT_H
