#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include "utama.h"
//#include "datasource.h"
#include "notification.h"
#include "mysql.h"
#include "config.h"
#include "monita_log.h"

class worker : public QObject
{
    Q_OBJECT
public:
    explicit worker(QObject *parent = 0);

    struct data_alarm dAlarm[MAX_DATA];
    int jml_data_alarm;

private:
    config cfg;
    monita_log log;
    redis rds;
//    dataSource source;
    QThread threadSource;
    notification *notf;
    QThread threadNotf;
    mysql db_mysql;
    QSqlDatabase db;

    void readCurrentValue();

    int time_period;
    QString redis_address;
    int redis_port;

    void processAlarm(int idx_alarm, int idx_rules, QStringList &alarm);
    void readAlarmParameter();
private slots:
    void doWork();

signals:
    void sendNotif(QStringList data, QDateTime dt, int index);
};

#endif // WORKER_H
