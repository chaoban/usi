#include <QCoreApplication>
#include "USIWRArgParser.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    USIWRArgParser argParser(argc, argv);

    argParser.doCommand();

    exit(0);

    return a.exec();
}
