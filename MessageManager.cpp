#include "MessageManager.h"

MessageManager::MessageManager(UserManager *userManager, PrivacyListManager *privacyListManager)
{
    m_userManager = userManager;
    m_privacyListManager = privacyListManager;
}

QByteArray MessageManager::parseMessage(QByteArray messageXML, QString messageFrom)
{
    QDomDocument document;
    document.setContent(messageXML);

    QDomElement messageNode = document.firstChildElement();
    QString from = messageNode.attribute("from", messageFrom);
    QString to = messageNode.attribute("to");
    QString type = messageNode.attribute("type");
    QString xmlLang = messageNode.attribute("xml:lang");

    /*if (!m_userManager->userExists(Utils::getBareJid(to)))
    {
        return Error::generateServiceUnavailableError("iq", from, to, Utils::generateId());
    }*/

    if ((type == "chat") || (type == "normal"))
    {
        messageNode.setAttribute("from", from);
        emit sigNewChatMessage(to, document.toByteArray());
    }
    return QByteArray();
}
