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

    static QByteArray generateError(QString by, QString stanzaType, QString errorType, QString errorCause,
                                    QString from, QString to, QString id, QDomElement request);
    static QByteArray generateError(QString by, QString stanzaType, QString errorType, QString errorCause,
                                    QString specificErrorCause, QString specificErrorXmlns, QString feature, QString from,
                                    QString to, QString id);
    static QByteArray generateStreamError(QString errorCause);
    static QByteArray generateSmError(QString childName);
    static QByteArray generateFailureError(QString xmlns, QString errorCause);
    static QByteArray generateInternalServerError();
};

#endif // ERROR_H
