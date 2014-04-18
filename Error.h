#ifndef ERROR_H
#define ERROR_H

#include <QDomElement>
#include <QDomDocument>
#include <QDebug>
#include "Utils.h"

class Error
{
public:
    Error();

    static QByteArray generateError(QString stanzaType, QString errorType, QString errorCause,
                                    QString from, QString to, QString id, QDomElement request);
    static QByteArray generateStreamError(QString errorCause);
    static QByteArray generateSmError(QString childName);
    static QByteArray generateSaslError(QString childName);
};

#endif // ERROR_H
