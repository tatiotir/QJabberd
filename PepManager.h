#ifndef PEPMANAGER_H
#define PEPMANAGER_H

#include "PubsubManager.h"

class PepManager : public PubsubManager
{
public:
    PepManager(RosterManager *rosterManager = 0, StorageManager *storageManager = 0);
    QByteArray PepManagerReply(QDomDocument document, QString iqFrom);
};

#endif // PEPMANAGER_H
