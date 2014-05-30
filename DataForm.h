#ifndef DATAFORM_H
#define DATAFORM_H

#include <QDomDocument>
#include <QDomElement>
#include <QMultiHash>

class DataForm
{
public:
    DataForm();

    static QByteArray getRegistrationForm(QString from, QString to, QString id);
    static QByteArray getPasswordChangeForm(QString from, QString to, QString id);
    static QMultiHash<QString, QString> parseDataForm(QDomElement xElement);
    static QDomDocument getRoomConfigForm(QString from, QString to, QString id);
};

#endif // DATAFORM_H
