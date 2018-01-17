#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QThread>

#include <QSignalMapper>
#include <QProcess>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include "modul/monita_log.h"
#include "util/config.h"
#include "3rdparty/qcron/src/qcron.hpp"
#include "modul/mysql.h"
#include "util/utama.h"
#include "controller/notification.h"

class scheduler : public QObject
{
    Q_OBJECT
public:
    explicit scheduler(QObject *parent = 0, notification *n = 0);
    ~scheduler();

    void doSetup(QThread &cThread);
    QJsonObject ObjectFromString(QString in);

private:
    void generate_report(int index, QString temp, QString conf, QString name, QString parameter);
    QVector<data_cron> m_data_cron;
    QList<QCron *> m_cron;
    QSignalMapper *SigMapCron_Activated;
    QSignalMapper *SigMapCron_Deactivated;
    QList<QNetworkAccessManager *> m_manager_cron;
//    QList<QNetworkReply *> m_reply_cron;
//    QSignalMapper *SigMapManager_Finished;
    monita_log log;
    config cfg;

    QString root_talisa_api;
    int debugData;
    int debugDatabase;

    mysql database;
    QSqlDatabase db;
//    QNetworkAccessManager *manager;

    notification *notf;
    QString email_sender;
    QString email_password;
    QString email_server;
    int     email_port;
    QString email_recipient;

    QString cron5RegEx;
    QString cron6RegEx;

private slots:
    void doWork();
    void onActivation(int index);
    void onDeactivation(int index);
    void replyFinished(QNetworkReply *reply);
//    void replyFinished(int);

};

#endif // SCHEDULER_H
