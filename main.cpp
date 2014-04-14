#include <QtWidgets/QApplication>
#include "server.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Server qjabberd;
    qjabberd.start();

    return app.exec();
}
