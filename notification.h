#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QThread>

#include <QWebSocketServer>
#include <QWebSocket>

#include "utama.h"
#include "mysql.h"
#include "config.h"
#include "monita_log.h"
#include "smtp.h"

class worker;

class notification : public QObject
{
    Q_OBJECT
public:
    explicit notification(worker *parent = 0);
    ~notification();

    void doSetup(QThread &cThread);

    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    QVector<QStringList> m_nama_titik_ukur;
    QVector<QStringList> m_titik_ukur;
    mysql db_mysql;
    QSqlDatabase db;
    redis rds;
    config cfg;
    monita_log log;

    int time_period;
    int webSocket_port;
    QString redis_address;
    int redis_port;

    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

    void WriteToJson(QJsonObject json, QDateTime dt, int index);
    void sendMail(QString username, QString password, QString server, int port, QString recipient, QString subject, QString message);

public slots:
    void doWork();
    void RedisToJson(QStringList data, QDateTime dt, int index);
    void mailSent(QString status);

Q_SIGNALS:
    void closed();

};

#endif // NOTIFICATION_H
