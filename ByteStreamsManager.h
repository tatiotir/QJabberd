#ifndef BYTESTREAMSMANAGER_H
#define BYTESTREAMSMANAGER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "Utils.h"
#include "Error.h"

class ByteStreamsManager : public QObject
{
    Q_OBJECT
public:
    ByteStreamsManager(QJsonObject *serverConfiguration = 0);

    QByteArray byteStreamManagerReply(QDomDocument document, QString iqFrom);
    QByteArray byteStreamManagerResult(QString from, QString to, QString id, QJsonObject proxyObject);

signals:

public slots:

private:
    QJsonObject getProxyObject(QString proxyJid);
    QJsonObject *m_serverConfiguration;

};

#endif // BYTESTREAMSMANAGER_H
