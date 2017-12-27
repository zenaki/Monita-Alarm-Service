#include "monita_log.h"

monita_log::monita_log()
{
//    buf_lay = (char *) malloc(1024);
//    buf_dateTime = (char *) malloc(32);
}

void monita_log::write(QString type, QString message, int debug)
{
    if (debug) {
        QString path = PATH_MONITA_LOG;
        QFile outFile(path);
        if (!outFile.exists()) {
            QDir dir;
            dir.mkpath(PATH_MONITA);
        }
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream ts(&outFile);
        message.prepend("::").prepend(type).prepend("::").prepend(QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss"));
//        qDebug() << message;
        printf("%s\n", message.toLatin1().data());
        ts << message << endl;
        outFile.close();
    } else {
        message.prepend("::").prepend(type).prepend("::").prepend(QDateTime::currentDateTime().toString("dd-MM-yyyy HH:mm:ss"));
//        qDebug() << message;
        printf("%s\n", message.toLatin1().data());
    }
}
