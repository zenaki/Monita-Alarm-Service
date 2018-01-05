#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QThread>

#include <QSignalMapper>
#include <QProcess>

#include "modul/monita_log.h"
#include "util/config.h"
#include "3rdparty/qcron/src/qcron.hpp"
#include "modul/mysql.h"
#include "util/utama.h"

class scheduler : public QObject
{
    Q_OBJECT
public:
    explicit scheduler(QObject *parent = 0);
    ~scheduler();

    void doSetup(QThread &cThread);
    QJsonObject ObjectFromString(QString in);

private:
    void generate_report(int index, QString temp, QString conf, QString name, QString parameter);
    QList<QCron *> m_cron;
    QSignalMapper *SigMapCron_Activated;
    QSignalMapper *SigMapCron_Deactivated;
    monita_log log;
    config cfg;

    int debugData;
    int debugDatabase;

    mysql database;
    QSqlDatabase db;

private slots:
    void doWork();
    void onActivation(int index);
    void onDeactivation(int index);
};

#endif // SCHEDULER_H
