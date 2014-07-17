#include <QtCore/QCoreApplication>
#include "Server.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Server qjabberd;
    qjabberd.start();

    return app.exec();
}
