#ifndef DATAFORMMANAGER_H
#define DATAFORMMANAGER_H

#include <QDomDocument>
#include <QDomElement>
#include <QMultiHash>
#include "Utils.h"
#include "DataFormField.h"

class DataFormManager
{
public:
    DataFormManager();

    static QDomElement generateDataForm(QString formType,
                                         QString title, QString instruction, QList<DataFormField> dataFormFields);
    static QDomDocument getRegistrationForm(QString id);
    static QDomDocument getPasswordChangeForm(QString from, QString to, QString id);
    static QMap<QString, QVariant> parseDataForm(QDomElement xElement);
    static QDomDocument getRoomConfigForm(QString type, QString from, QString to, QString id, QMap<QString, QVariant> dataFormValue);
    static QDomDocument getRoomRegistrationForm(QString from, QString to, QString id, QString roomName);
    static QDomDocument getRoomVoiceRequestForm(QString from, QString to, QString id, QString role, QString jid, QString roomnick);
};

#endif // DATAFORMMANAGER_H
