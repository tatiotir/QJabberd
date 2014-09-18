#ifndef INTERESTEDPEP_H
#define INTERESTEDPEP_H

#include <QString>

class InterestedPep
{
public:
    InterestedPep(QString jid, bool mood, bool tune, bool physloc, bool geoloc);
    InterestedPep();

    QString jid() const;
    void setJid(const QString &jid);

    bool mood() const;
    void setMood(bool mood);

    bool tune() const;
    void setTune(bool tune);

    bool physloc() const;
    void setPhysloc(bool physloc);

    bool geoloc() const;
    void setGeoloc(bool geoloc);

private:
    QString m_jid;
    bool m_mood;
    bool m_tune;
    bool m_physloc;
    bool m_geoloc;
};

#endif // INTERESTEDPEP_H
