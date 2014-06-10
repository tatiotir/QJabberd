#include "InbandByteStreamsManager.h"

InbandByteStreamsManager::InbandByteStreamsManager()
{
}

QByteArray InbandByteStreamsManager::inbandByteStreamsManagerReply(QDomDocument document, QString iqFrom)
{
    emit sigInbandByteStreamRequest(document.firstChildElement().attribute("to"), document);
}
