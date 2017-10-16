#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QObject>
#include <QThread>

#include <QWebSocketServer>
#include <QWebSocket>

#include "utama.h"

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
    redis rds;

    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

    void WriteToJson(QJsonObject json, QDateTime dt, int index);

public slots:
    void doWork();
    void RedisToJson(QStringList data, QDateTime dt, int index);

Q_SIGNALS:
    void closed();
};

#endif // NOTIFICATION_H
