#ifndef INBANDBYTESTREAMSMANAGER_H
#define INBANDBYTESTREAMSMANAGER_H

#include <QDebug>
#include <QString>
#include <QDomElement>
#include <QDomNodeList>

class InbandByteStreamsManager : public QObject
{
    Q_OBJECT
public:
    InbandByteStreamsManager();

    QByteArray inbandByteStreamsManagerReply(QDomDocument document, QString iqFrom);

signals:
    void inbandByteStreamRequest(QString to, QByteArray data);


};

#endif // INBANDBYTESTREAMSMANAGER_H
