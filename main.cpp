#include <QCoreApplication>
#include "controller/worker.h"

#define VERSION "BETA TEST"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc == 2) {
        QString arg1(argv[1]);
        if (arg1 == QLatin1String("-v") ||
            arg1 == QLatin1String("--version")) {
            QString compilationTime = QString("%1 %2").arg(__DATE__).arg(__TIME__);
            QString version = VERSION;
            printf("Monita Service Version:  %s\nPT. DaunBiru Engineering\nwww.daunbiru.com\n\n"
                   "build on: %s (UTC+7)\n",
                   version.toUtf8().data(),
                   compilationTime.toUtf8().data());
        }
        return 0;
    } else {
        printf(":: Monita Alarm Service Started ::\n");
        worker w;
        return a.exec();
    }

//    return 0;
}
