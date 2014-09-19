QT       += core xml network sql

TARGET = QJabberd

SOURCES += \
    EntityTimeManager.cpp \
    Connection.cpp \
    ConnectionManager.cpp \
    Contact.cpp \
    IqManager.cpp \
    LastActivityManager.cpp \
    LocalStorage.cpp \
    MessageManager.cpp \
    PresenceManager.cpp \
    PrivacyListItem.cpp \
    PrivacyListManager.cpp \
    Sasl.cpp \
    Server.cpp \
    ServiceDiscoveryManager.cpp \
    Storage.cpp \
    StorageManager.cpp \
    Stream.cpp \
    StreamManager.cpp \
    UserManager.cpp \
    Utils.cpp \
    VcardManager.cpp \
    Error.cpp \
    main.cpp \
    MetaContact.cpp \
    RosterManager.cpp \
    StreamNegotiationManager.cpp \
    PrivateStorageManager.cpp \
    User.cpp \
    OfflineMessageManager.cpp \
    StreamNegotiationData.cpp \
    InbandByteStreamsManager.cpp \
    OobDataManager.cpp \
    PgSqlStorage.cpp \
    BlockingCommandManager.cpp \
    MucManager.cpp \
    Occupant.cpp \
    DataFormManager.cpp \
    ByteStreamsManager.cpp \
    DataFormField.cpp \
    BoshManager.cpp \
    BoshSession.cpp \
    BoshSessionManager.cpp \
    PubsubManager.cpp \
    PubsubItem.cpp \
    NodeSubscriber.cpp \
    InterestedPep.cpp \
    MySqlStorage.cpp

HEADERS += \
    EntityTimeManager.h \
    Connection.h \
    ConnectionManager.h \
    Contact.h \
    Error.h \
    IqManager.h \
    LastActivityManager.h \
    LocalStorage.h \
    MessageManager.h \
    PresenceManager.h \
    Sasl.h \
    Server.h \
    ServiceDiscoveryManager.h \
    Storage.h \
    Stream.h \
    StreamManager.h \
    UserManager.h \
    Utils.h \
    VcardManager.h \
    PrivacyListItem.h \
    PrivacyListManager.h \
    StorageManager.h \
    MetaContact.h \
    RosterManager.h \
    StreamNegotiationManager.h \
    PrivateStorageManager.h \
    User.h \
    OfflineMessageManager.h \
    StreamNegotiationData.h \
    MySqlStorage.h \
    InbandByteStreamsManager.h \
    OobDataManager.h \
    PgSqlStorage.h \
    BlockingCommandManager.h \
    MucManager.h \
    Occupant.h \
    DataFormManager.h \
    ByteStreamsManager.h \
    DataFormField.h \
    BoshManager.h \
    BoshSession.h \
    BoshSessionManager.h \
    PubsubManager.h \
    PubsubItem.h \
    NodeSubscriber.h \
    InterestedPep.h

RESOURCES += \
    qjabberd_resources.qrc

RC_FILE += \
    ressource.rc

OTHER_FILES += \
    ressource.rc
