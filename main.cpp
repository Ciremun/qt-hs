#include <QCoreApplication>

#include "hs_server.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    auto server = new TCPServer();
    return a.exec();
}
