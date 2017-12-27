#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QThread>

#include "modul/monita_log.h"
#include "3rdparty/qcron/src/qcron.hpp"

class scheduler : public QObject
{
    Q_OBJECT
public:
    explicit scheduler(QObject *parent = 0);
    ~scheduler();

    void doSetup(QThread &cThread);

    QCron *cron;
    monita_log log;
private slots:
    void doWork();
    void onActivation();
    void onDeactivation();
};

#endif // SCHEDULER_H
