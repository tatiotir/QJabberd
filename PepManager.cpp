#include "PepManager.h"

PepManager::PepManager(RosterManager *rosterManager, StorageManager *storageManager) :
    PubsubManager(rosterManager, storageManager)
{
}

QByteArray PepManager::PepManagerReply(QDomDocument document, QString iqFrom)
{
    QString from = document.documentElement().attribute("from", iqFrom);
    QString id = document.documentElement().attribute("id", Utils::generateId());
    QString xmlns = document.documentElement().firstChildElement().attribute("xmlns");

    QString type = document.documentElement().attribute("type");
    QDomElement pubsubChild = document.documentElement().firstChildElement().firstChildElement();

    if (type == "set")
    {
        QString node = pubsubChild.attribute("node");
        QMultiMap<QString, QVariant> dataFormValues = DataFormManager::parseDataForm(document.documentElement().firstChildElement().elementsByTagName("configure").item(0).firstChildElement());
        createNode("", node, Utils::getBareJid(from), dataFormValues);
    }
}

