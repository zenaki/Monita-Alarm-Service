#include "notification.h"
#include "worker.h"

notification::notification(worker *parent) : QObject(parent)
{
    QStringList temp = cfg.read("CONFIG");
    time_period = temp.at(0).toInt();
    webSocket_port = temp.at(1).toInt();
    temp = cfg.read("REDIS");
    redis_address = temp.at(0);
    redis_port = temp.at(1).toInt();

    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);

    bool webSocketServer = false;
    while (!webSocketServer) {
        if (m_pWebSocketServer->listen(QHostAddress::Any, webSocket_port)) {
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
    t->start(time_period);
}

void notification::doWork()
{
    QDateTime dt = QDateTime::currentDateTime();
    QStringList request = rds.reqRedis("hlen monita_alarm_service:notification", redis_address, redis_port);
//    log.write("Redis",request.at(0) + " Data ..",
//              monita_cfg.config.at(7).toInt());
    int redis_len = request.at(0).toInt();
    if (redis_len > 0) {
        log.write("ALARM", "GET Notifications", 0);
//        qDebug() << "XXX GET Notifications XXX " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
        request = rds.reqRedis("hgetall monita_alarm_service:notification", redis_address, redis_port, redis_len*2);
        for (int i = 0; i < m_clients.length(); i++) {
            this->RedisToJson(request, dt, i);
        }
        QString mysql_command, status;
        QStringList notifParam;
        for (int i = 0; i < request.length(); i+=2) {
            QStringList list_temp1 = request.at(i).split(";");
            QStringList list_temp2 = request.at(i+1).split(";");
            status = list_temp2[1].replace("_", " ");
            if (i == request.length()-2) {
                mysql_command = mysql_command +
                        "(\\'" + list_temp1.at(0) +
                        "\\',\\'" + list_temp1.at(1) +
                        "\\',\\'" + list_temp1.at(2) +
                        "\\',\\'" + list_temp2.at(0) +
                        "\\',\\'" + status + "\\')";
            } else {
                mysql_command = mysql_command +
                        "(\\'" + list_temp1.at(0) +         // ID Alarm
                        "\\',\\'" + list_temp1.at(1) +      // ID Titik Ukur
                        "\\',\\'" + list_temp1.at(2) +      // Event Time
                        "\\',\\'" + list_temp2.at(0) +      // Current Value
                        "\\',\\'" + status + "\\'),";       // Status
            }
            notifParam.append(list_temp1.at(0));
            notifParam.append(list_temp1.at(1));
            notifParam.append(list_temp1.at(2));
            notifParam.append(list_temp2.at(0));
            notifParam.append(status);
//            if (list_temp1.at(1) == "848019") {
//                qDebug() << "Test";
//            }
            sendNotification(notifParam);
            notifParam.clear();
        }
        db_mysql.write_alarm_history(db, mysql_command, "Test", 0);
        request = rds.reqRedis("del monita_alarm_service:notification", redis_address, redis_port);
    }
}

void notification::RedisToJson(QStringList data, QDateTime dt, int index)
{
    QJsonObject json;
    QJsonObject AlaMonObject;
    QJsonArray AlaMonArray;
    QString status;

    for (int i = 0; i < data.length(); i+=2) {
        QStringList list_temp1 = data.at(i).split(";");
        QStringList list_temp2 = data.at(i+1).split(";");
        status = list_temp2[1].replace("_", " ");
        for (int j = 0; j < m_titik_ukur[index].length(); j++) {
            if (list_temp1.at(1) == m_titik_ukur[index][j]) {
                if (list_temp2.at(1) != "NORMAL") {
                    json["alarm_id"] = list_temp1.at(0);
                    json["nama_titik"] = m_nama_titik_ukur[index][j];
                    json["titik_ukur"] = list_temp1.at(1);
                    json["time"] = list_temp1.at(2);
                    json["current_value"] = list_temp2.at(0);
                    json["status"] = status;
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
    QString path = QString(PATH_MONITA) + "/NotMon_" + dt.date().toString("yyyyMMdd") + ".json";
    QFile visual_json_file(path);
    if (!visual_json_file.exists()) {
        QDir dir;
        dir.mkpath(PATH_MONITA);
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
    visual_json_file.remove();
}

void notification::sendNotification(QStringList notifParam)
{
    QString index, titik_ukur;
    for (int i = 0; i < notifParam.length(); i+=5) {
        if (titik_ukur.isEmpty()) {
            titik_ukur = notifParam.at(i+1);
            index = QString::number(i);
        } else {
            titik_ukur = titik_ukur + "," + notifParam.at(1);
            index = index + "," + QString::number(i);
        }
    }

    QStringList email_config = cfg.read("EMAIL");
    QString email_sender = email_config.at(0);
    QString email_password = email_config.at(1);
    QString email_server = email_config.at(2);
    int email_port = email_config.at(3).toInt();

    QStringList listEmail = db_mysql.read_email(db, titik_ukur, "MySQL", 0);
    QStringList listTempIndex = index.split(",");
    QStringList listTempTitikUkur = titik_ukur.split(",");
    for (int i = 0; i < listEmail.length(); i+=2) {
        for (int j = 0; j < listTempTitikUkur.length(); j++) {
            if (listTempTitikUkur.at(j) == listEmail.at(i)) {
                sendMail(
                    email_sender,
                    email_password,
                    email_server,
                    email_port,
                    listEmail.at(i+1),
                    "Alarm from Measurement Point : " +
                            listEmail.at(0),
                    "<font size=\"4\" color=\"red\"><b>"
                    "\n</br>Measurement Point : " +
                            notifParam.at(listTempIndex.at(j).toInt()+1) +
                    "\n</br>Time              : " +
                            notifParam.at(listTempIndex.at(j).toInt()+2) +
                    "\n</br>Value             : " +
                            notifParam.at(listTempIndex.at(j).toInt()+3) +
                    "\n</br>Status            : " +
                            notifParam.at(listTempIndex.at(j).toInt()+4) +
                    "\n</b></font>",
                    QStringList()
                );
            }
        }
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
        if (data.at(0) == "usr") {
            for (int i = 0; i < m_clients.length(); i++) {
                if (m_clients.at(i) == pClient) {
//                    QStringList temp_nama, temp_titik_ukur;
//                    temp_nama = m_nama_titik_ukur.at(i);
//                    temp_titik_ukur = m_titik_ukur.at(i);

                    QStringList dataAlarm = db_mysql.read_titik_ukur(db, data.at(1).toInt(), data.at(0), 0);
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
    Q_UNUSED(message);
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

void notification::sendMail(
        QString     username,
        QString     password,
        QString     server,
        int         port,
        QString     recipient,
        QString     subject,
        QString     message,
        QStringList attachPath)
{
//    SmtpClient smtp(server, port, SmtpClient::SslConnection);
    SmtpClient smtp(server, port, SmtpClient::TlsConnection);
    smtp.setUser(username);
    smtp.setPassword(password);

    MimeMessage MimeMSG;

    EmailAddress sender(username, "Monita - Server");
    MimeMSG.setSender(&sender);

    EmailAddress to(recipient, "Monita - Client");
    MimeMSG.addRecipient(&to);

    MimeMSG.setSubject(subject);

    MimeText text;
    text.setText(message);
    MimeMSG.addPart(&text);

    for (int i = 0; i < attachPath.length(); i++) {
        QFile path(attachPath.at(i));
        if (path.exists()) {
//            MimeMSG.addPart(new MimeAttachment(path));
            MimeMSG.addPart(new MimeAttachment(new QFile(attachPath.at(i))));
        }
    }

    if (!smtp.connectToHost()) {
        log.write("Email", "Failed to connect to host!", 0);
        return;
    }

    if (!smtp.login()) {
        log.write("Email", "Failed to login!", 0);
        return;
    }

    if (!smtp.sendMail(MimeMSG)) {
        log.write("Email", "Failed to send mail!", 0);
        return;
    } else {
        log.write("Email", "Sending to " + recipient, 0);
    }

    smtp.quit();
}

void notification::mailSent(QString status)
{
//    if(status == "Message sent")
//        qDebug() << "Message Sent ..!!";
    qDebug() << status;
}
