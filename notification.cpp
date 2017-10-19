#include "notification.h"
#include "worker.h"

notification::notification(worker *parent) : QObject(parent)
{
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);

    bool webSocketServer = false;
    while (!webSocketServer) {
        if (m_pWebSocketServer->listen(QHostAddress::Any, 2345)) {
//            log.write("WebSocket","Server listening on port : " + QString::number(port),
//                      monita_cfg.config.at(7).toInt());
            connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &notification::onNewConnection);
            connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &notification::closed);
            webSocketServer = true;
        } else {
            m_pWebSocketServer->close();
        }
    }

    db = db_mysql.connect_db("notification");
}

notification::~notification()
{
    if (m_pWebSocketServer->isListening()) {
        m_pWebSocketServer->close();
        qDeleteAll(m_clients.begin(), m_clients.end());
    }
}

void notification::doSetup(QThread &cThread)
{
    connect(&cThread, SIGNAL(started()), this, SLOT(doWork()));

//    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);

//    bool webSocketServer = false;
//    while (!webSocketServer) {
//        if (m_pWebSocketServer->listen(QHostAddress::Any, 2345)) {
////            log.write("WebSocket","Server listening on port : " + QString::number(port),
////                      monita_cfg.config.at(7).toInt());
//            connect(m_pWebSocketServer, &QWebSocketServer::newConnection,this, &notification::onNewConnection);
//            connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &notification::closed);
//            webSocketServer = true;
//        } else {
//            m_pWebSocketServer->close();
//        }
//    }

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(TIME_PERIOD);
}

void notification::doWork()
{
//    QDateTime dt = QDateTime::currentDateTime();
//    QStringList request = rds.reqRedis("hlen monita_alarm_service:test_alarm", REDIS_ADDRESS, REDIS_PORT);
////    log.write("Redis",request.at(0) + " Data ..",
////              monita_cfg.config.at(7).toInt());
//    int redis_len = request.at(0).toInt();
//    if (redis_len > 0) {
//        request = rds.reqRedis("hgetall monita_alarm_service:test_alarm", REDIS_ADDRESS, REDIS_PORT, redis_len*2);

//        for (int i = 0; i < m_clients.length(); i++) {
//            this->RedisToJson(request, dt, i);
//        }
////        request = rds.reqRedis("del monita_service:vismon", address, port, redis_len*2);
//    }
}

void notification::RedisToJson(QStringList data, QDateTime dt, int index)
{
    QJsonObject json;
    QJsonObject AlaMonObject;
    QJsonArray AlaMonArray;

    for (int i = 0; i < data.length(); i+=5) {
        for (int j = 0; j < m_titik_ukur[index].length(); j++) {
            if (data.at(i+1) == m_titik_ukur[index][j]) {
                if (data.at(i+4) != "NORMAL") {
                    json["alarm_id"] = data.at(i);
                    json["nama_titik"] = m_nama_titik_ukur[index][j];
                    json["titik_ukur"] = data.at(i+1);
                    json["time"] = data.at(i+2);
                    json["current_value"] = data.at(i+3);
                    json["status"] = data.at(i+4);
                    AlaMonArray.append(json);
                }
            }
        }
    }
    AlaMonObject["monita_alarm"] = AlaMonArray;
    this->WriteToJson(AlaMonObject, dt, index);
}

void notification::WriteToJson(QJsonObject json, QDateTime dt, int index)
{
    QString path = ".MonAlaSerConfig/NotMon_" + dt.date().toString("yyyyMMdd") + ".json";
    QFile visual_json_file(path);
    if (!visual_json_file.exists()) {
        QDir dir;
        dir.mkpath(".MonAlaSerConfig");
    }
    if (visual_json_file.open(QIODevice::ReadWrite|QIODevice::Truncate)) {
        QJsonDocument saveDoc(json);
        visual_json_file.write(saveDoc.toJson());
    }
    visual_json_file.close();
    if (visual_json_file.open(QIODevice::ReadWrite)) {
        QByteArray readFile = visual_json_file.readAll();
        m_clients.at(index)->sendTextMessage(readFile);
//        for (int i = 0; i < m_clients.length(); i++) {
//            if (m_clients.at(i)->isValid()) {
//                m_clients.at(i)->sendTextMessage(readFile);
//            }
//        }
    }
}

void notification::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &notification::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &notification::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &notification::socketDisconnected);

    pSocket->ignoreSslErrors();
//    log.write("WebSocket","New Client : " + pSocket->localAddress().toString() + ":" + pSocket->localPort(),
//              monita_cfg.config.at(7).toInt());
//    pSocket->sendTextMessage("Berhasil Connect cuy ..");
    m_clients << pSocket;

    QStringList temp_nama, temp_titik_ukur;
    m_nama_titik_ukur << temp_nama;
    m_titik_ukur << temp_titik_ukur;
}

void notification::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
//    log.write("WebSocket","Client : " + pClient->localAddress().toString() + " Message received : " + message,
//              monita_cfg.config.at(7).toInt());
//    if (pClient) {pClient->sendTextMessage(message);}
    if (!message.isEmpty()) {
        QStringList data = message.split(":");
        if (data.at(0) == "id") {
            for (int i = 0; i < m_clients.length(); i++) {
                if (m_clients.at(i) == pClient) {
//                    QStringList temp_nama, temp_titik_ukur;
//                    temp_nama = m_nama_titik_ukur.at(i);
//                    temp_titik_ukur = m_titik_ukur.at(i);

                    QStringList dataAlarm = db_mysql.read_titik_ukur(db, data.at(1).toInt(), "Test", 0);
                    for (int j = 0; j < dataAlarm.length(); j+=2) {
//                        temp_nama.append(dataAlarm.at(j));
//                        temp_titik_ukur.append(dataAlarm.at(j+1));
                        m_nama_titik_ukur[i].append(dataAlarm.at(j));
                        m_titik_ukur[i].append(dataAlarm.at(j+1));
                    }
                    break;
                }
            }
        }
    }

}

void notification::processBinaryMessage(QByteArray message)
{
//    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
//    log.write("WebSocket","Client : " + pClient->localAddress().toString() + " Binary Message received : " + message,
//              monita_cfg.config.at(7).toInt());
//    if (pClient) {pClient->sendBinaryMessage(message);}

}

void notification::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
//    log.write("WebSocket","Socket Disconnect : " + pClient->localAddress().toString() + ":" + pClient->localPort(),
//              monita_cfg.config.at(7).toInt());
    if (pClient) {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}
