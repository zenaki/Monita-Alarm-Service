#ifndef WORKER_H
#define WORKER_H

#include <QObject>

#include "utama.h"
#include "datasource.h"
#include "notification.h"
#include "mysql.h"

class worker : public QObject
{
    Q_OBJECT
public:
    explicit worker(QObject *parent = 0);

    struct data_alarm dAlarm[MAX_DATA];
    int jml_data_alarm;

private:
    redis rds;
    dataSource source;
    QThread threadSource;
    notification *notf;
    QThread threadNotf;
    mysql db_mysql;
    QSqlDatabase db;

    void readCurrentValue();

private slots:
    void doWork();

signals:
    void sendNotif(QStringList data, QDateTime dt, int index);
};

#endif // WORKER_H
