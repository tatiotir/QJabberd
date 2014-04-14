#include "StreamNegotiationData.h"

StreamNegotiationData::StreamNegotiationData()
{
    m_firstFeatureProceed = false;
    m_secondFeatureProceed = false;
    m_secondChallengeGenerated = false;
    m_bindFeatureProceed = false;
    m_host = "";
    m_clientFirstResponseDataMap = QMultiHash<QString, QByteArray>();
}

bool StreamNegotiationData::secondFeatureProceed()
{
    return m_secondFeatureProceed;
}

bool StreamNegotiationData::firstFeatureProceed()
{
    return m_firstFeatureProceed;
}

bool StreamNegotiationData::bindFeatureProceed()
{
    return m_bindFeatureProceed;
}

bool StreamNegotiationData::secondChallengeGenerated()
{
    return m_secondChallengeGenerated;
}

QString StreamNegotiationData::getHost()
{
    return m_host;
}

QMultiHash<QString, QByteArray> StreamNegotiationData::getClientFirstResponseDataMap()
{
    return m_clientFirstResponseDataMap;
}

void StreamNegotiationData::setSecondFeatureProceed(bool val)
{
    m_secondFeatureProceed = val;
}

void StreamNegotiationData::setBindFeatureProceed(bool val)
{
    m_bindFeatureProceed = val;
}

void StreamNegotiationData::setFirstFeatureProceed(bool val)
{
    m_firstFeatureProceed = val;
}

void StreamNegotiationData::setSecondChallengeGenerated(bool val)
{
    m_secondChallengeGenerated = val;
}

void StreamNegotiationData::setHost(QString host)
{
    m_host = host;
}

void StreamNegotiationData::setClientFirstResponseDataMap(QMultiHash<QString, QByteArray> dataMap)
{
    m_clientFirstResponseDataMap = dataMap;
}
