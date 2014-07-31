QJabberd Instant Messaging Server
=================================

QJabberd is an XMPP server write in Qt.

Quickstart guide
=================

0. Requirements
===============

To compile QJabberd you need : 

- Qt 5

1. Compile and install QJabberd
===============================

To compile QJabberd, install Qt 5 with Qt Creator. Open the file "QJabberd.pro" (Qt Project file).
Just compile the project in debug or release mode and enjoy.

1.1 Use MySql or PostGreSql Storage
===================================

To use mysql or postgresql as backend storage, you must first build the "qsqlmysql" and "qsqlpsql" driver in qt.
After that you can edit the file "qjabberd.config" (QJabberd configuration file) and change the parameter "storageType" to "PgSql" or "MySql". Now you must give the "host", "port", "username", "password" and "database" to use for MySql or PostGreSql connection (See the paramater "MySql:" and "PgSql:" in the QJabberd configuration file).

1.2 Start QJabberd

To start QJabberd, just run the binary file generate after the compilation process.
