#include "OobDataManager.h"

OobDataManager::OobDataManager()
{
}

QByteArray OobDataManager::oobDataManagerReply(QByteArray iqXML, QString iqFrom)
{
    QDomDocument document;
    document.setContent(iqXML);

    emit sigOobRequest(document.firstChildElement().attribute("to"), iqXML);
    return QByteArray();
}
