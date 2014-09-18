#include "InterestedPep.h"

InterestedPep::InterestedPep(QString jid, bool mood, bool tune, bool physloc, bool geoloc)
{
    m_jid = jid;
    m_mood = mood;
    m_tune = tune;
    m_physloc = physloc;
    m_geoloc = geoloc;
}

InterestedPep::InterestedPep()
{
    m_jid = "";
    m_mood = false;
    m_tune = false;
    m_physloc = false;
    m_geoloc = false;
}

QString InterestedPep::jid() const
{
    return m_jid;
}

void InterestedPep::setJid(const QString &jid)
{
    m_jid = jid;
}
bool InterestedPep::mood() const
{
    return m_mood;
}

void InterestedPep::setMood(bool mood)
{
    m_mood = mood;
}
bool InterestedPep::tune() const
{
    return m_tune;
}

void InterestedPep::setTune(bool tune)
{
    m_tune = tune;
}
bool InterestedPep::physloc() const
{
    return m_physloc;
}

void InterestedPep::setPhysloc(bool physloc)
{
    m_physloc = physloc;
}
bool InterestedPep::geoloc() const
{
    return m_geoloc;
}

void InterestedPep::setGeoloc(bool geoloc)
{
    m_geoloc = geoloc;
}





