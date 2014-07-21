#ifndef ENTITYTIMEMANAGER_H
#define ENTITYTIMEMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include "UserManager.h"

class EntityTimeManager : public QObject
{
    Q_OBJECT
public:
    explicit EntityTimeManager(QObject *parent = 0);

    QByteArray entityTimeManagerReply(QDomDocument document, QString iqFrom);
    QByteArray entityTimeMananerResult(QString from, QString to, QString id, QString tzo, QString utc);

signals:
    void sigSendReceiptRequest(QString to, QByteArray data);

public slots:

private:
    UserManager *m_userManager;

};

#endif // ENTITYTIMEMANAGER_H
