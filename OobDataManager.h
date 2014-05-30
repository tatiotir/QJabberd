#ifndef OOBDATAMANAGER_H
#define OOBDATAMANAGER_H

#include <QDebug>
#include <QString>
#include <QDomElement>
#include <QDomNodeList>

class OobDataManager : public QObject
{
    Q_OBJECT
public:
    OobDataManager();

    QByteArray oobDataManagerReply(QDomDocument document, QString iqFrom);

signals:
    void sigOobRequest(QString to, QByteArray request);
};

#endif // OOBDATAMANAGER_H
