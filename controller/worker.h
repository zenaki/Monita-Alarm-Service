#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include "util/utama.h"
//#include "datasource.h"
#include "controller/notification.h"
#include "modul/mysql.h"
#include "util/config.h"
#include "modul/monita_log.h"
//#include "processalarm.h"
#include "controller/scheduler.h"

class worker : public QObject
{
    Q_OBJECT
public:
    explicit worker(QObject *parent = 0);

    struct data_alarm dAlarm;
    QList<data_alarm > DataAlarm;
////    ProcessAlarm *procAlarm;
//    QList<ProcessAlarm *> AlarmObj;
////    QThread *threadAlarm;
//    QList<QThread *> AlarmThread;
    int jml_data_alarm;

private:
    config cfg;
    monita_log log;
    redis rds;
//    dataSource source;
//    QThread threadSource;
    notification *notf;
    QThread threadNotf;
    scheduler *schd;
    QThread threadSchd;

    mysql db_mysql;
    QSqlDatabase db;

    void readCurrentValue();

    int time_period;

    QString redis_address;
    int     redis_port;

    QString email_sender;
    QString email_password;
    QString email_server;
    int     email_port;
    QString email_recipient;

    void processAlarm(int idx_rules, QStringList &alarm);
    void readAlarmParameter();
private slots:
    void doWork();

signals:
    void sendNotif(QStringList data, QDateTime dt, int index);
};

#endif // WORKER_H
