#include "OobDataManager.h"

OobDataManager::OobDataManager()
{
}

QByteArray OobDataManager::oobDataManagerReply(QDomDocument document, QString iqFrom)
{
    emit sigOobRequest(document.firstChildElement().attribute("to"), document.toByteArray());
    return QByteArray();
}
