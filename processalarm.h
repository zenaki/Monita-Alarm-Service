#ifndef PROCESSALARM_H
#define PROCESSALARM_H

#include <QObject>
#include <QThread>

#include "utama.h"
#include "config.h"
#include "monita_log.h"

class ProcessAlarm : public QObject
{
    Q_OBJECT

public:
    explicit ProcessAlarm(QObject *parent = 0);

    void doSetup(QThread *cThread, struct data_alarm DataAlarm);

    struct data_alarm dAlarm;
    config cfg;
    redis rds;
    monita_log log;

    int time_period;
    QString redis_address;
    int redis_port;

    void processAlarm(int idx_rules, QStringList &alarm);
    void readCurrentValue();

public slots:
    void doWork();

};

#endif // PROCESSALARM_H
