#include "VcardManager.h"

VCardManager::VCardManager(StorageManager *storageManager)
{
    m_storageManager = storageManager;
}

QByteArray VCardManager::vCardManagerReply(QDomDocument document, QString iqFrom)
{
    QDomElement iq = document.documentElement();
    QString from = iq.attribute("from", iqFrom);
    QString to = iq.attribute("to");
    QString id = iq.attribute("id", Utils::generateId());

    if (iq.attribute("type") == "get")
    {
        if (to.isEmpty())
        {
            if (vCardExist(Utils::getBareJid(from)))
            {
                QString vCard = getVCard(Utils::getBareJid(from));
                return generateVCardResult("", from, id, vCard);
            }
            else
            {
                return Error::generateError("iq", "cancel", "item-not-found", from, "", id, iq.firstChildElement());
            }
        }
        else
        {
            if (vCardExist(Utils::getBareJid(to)))
            {
                QString vCard = getVCard(Utils::getBareJid(to));
                return generateVCardResult(to, from, id, vCard);
            }
            else
            {
                return Error::generateError("iq", "cancel", "item-not-found", from, "", id, iq.firstChildElement());
            }
        }

    }
    else if (iq.attribute("type") == "set")
    {
        if (from != iqFrom)
        {
            return Error::generateError("iq", "cancel", "not-allowed", "", from, id, QDomElement());
        }
        else
        {
//            iqXML.remove(iqXML.indexOf("</iq>"), 5);
//            int len = iqXML.indexOf(">") + 1;
//            iqXML.remove(0, len);
            QString vCardInfos;
            QTextStream stream(&vCardInfos);
            QDomNode vCardNode = document.documentElement().firstChildElement();

            vCardNode.save(stream, 0);
            updateVCard(Utils::getBareJid(from), vCardInfos);

            return generateIQResult(from, id);
        }
    }
}

QByteArray VCardManager::generateVCardResult(QString from, QString to, QString id, QString vCard)
{
    QDomDocument document;

    QDomDocument vCardDocument;
    vCardDocument.setContent(vCard);

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    if (!from.isEmpty())
        iq.setAttribute("from", from);

    iq.appendChild(vCardDocument.documentElement());
    document.appendChild(iq);

    // Request Acknowledgment of receipt
    sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}

QByteArray VCardManager::generateIQResult(QString to, QString id)
{
    QDomDocument document;

    QDomElement iq = document.createElement("iq");
    iq.setAttribute("to", to);
    iq.setAttribute("id", id);
    iq.setAttribute("type", "result");

    document.appendChild(iq);

    // Request Acknowledgment of receipt
    sigSendReceiptRequest(to, document.toByteArray());
    return document.toByteArray();
}

QString VCardManager::getVCard(QString jid)
{
    return m_storageManager->getStorage()->getVCard(jid);
}

bool VCardManager::updateVCard(QString jid, QString vCardInfos)
{
    return m_storageManager->getStorage()->updateVCard(jid, vCardInfos);
}

bool VCardManager::vCardExist(QString jid)
{
    return m_storageManager->getStorage()->vCardExist(jid);
}
