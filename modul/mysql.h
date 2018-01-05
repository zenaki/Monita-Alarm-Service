#ifndef MYSQL_H
#define MYSQL_H

#include "util/utama.h"

class mysql
{
public:
    mysql();

    QString host;
    QString db_name;
    QString user_name;
    QString password;

    QSqlDatabase connect_db(QString name);
    void close(QSqlDatabase db);

    monita_log log;

    QStringList read_titik_ukur(QSqlDatabase db, int id_user, QString type, int debug);
    QStringList read_data_alarm(QSqlDatabase db, QString type, int debug);

    void write_alarm_history(QSqlDatabase db, QString data, QString type, int debug);
    QStringList read_email(QSqlDatabase db, QString titik_ukur, QString type, int debug);
    QStringList read_cron(QSqlDatabase db, QString type, int debug);
};

#endif // MYSQL_H
