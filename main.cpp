#include <QCoreApplication>
#include "worker.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    worker w;
    return a.exec();
}
