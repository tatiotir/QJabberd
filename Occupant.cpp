#include "Occupant.h"

Occupant::Occupant(QString jid, QString mucJid, QString affiliation, QString role, QString status, QString show)
{
    m_jid = jid;
    m_affiliation = affiliation;
    m_role = role;
    m_mucJid = mucJid;
    m_status = status;
    m_show = show;
}

Occupant::Occupant()
{
    m_jid = "";
    m_affiliation = "none";
    m_role = "none";
    m_mucJid = "";
    m_status = "";
    m_show = "";
}

QString Occupant::jid() const
{
    return m_jid;
}

void Occupant::setJid(const QString &jid)
{
    m_jid = jid;
}
QString Occupant::affiliation() const
{
    return m_affiliation;
}

void Occupant::setAffiliation(const QString &affiliation)
{
    m_affiliation = affiliation;
}
QString Occupant::role() const
{
    return m_role;
}

void Occupant::setRole(const QString &role)
{
    m_role = role;
}

QString Occupant::mucJid() const
{
    return m_mucJid;
}

void Occupant::setMucJid(const QString &mucJid)
{
    m_mucJid = mucJid;
}

QJsonObject Occupant::toJsonObject()
{
    QJsonObject object;
    object.insert("jid", m_jid);
    object.insert("mucJid", m_mucJid);
    object.insert("affiliation", m_affiliation);
    object.insert("role", m_role);
    object.insert("status", m_status);
    object.insert("show", m_show);
    return object;
}

Occupant Occupant:: fromJsonObject(QJsonObject object)
{
    return Occupant(object.value("jid").toString(),
                    object.value("mucJid").toString(),
                    object.value("affiliation").toString(),
                    object.value("role").toString(),
                    object.value("status").toString(),
                    object.value("show").toString());
}
QString Occupant::status() const
{
    return m_status;
}

void Occupant::setStatus(const QString &status)
{
    m_status = status;
}
QString Occupant::show() const
{
    return m_show;
}

void Occupant::setShow(const QString &show)
{
    m_show = show;
}


