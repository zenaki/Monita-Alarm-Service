#include "worker.h"

worker::worker(QObject *parent) : QObject(parent)
{       
#ifdef SIMULATION
    time_period = TIME_PERIOD;
    redis_address = REDIS_ADDRESS;
    redis_port = REDIS_PORT;

    QStringList id_tu;
    int id = 0;
    jml_data_alarm = 0;
//    rds.reqRedis("del monita_alarm_service:test_rule", REDIS_ADDRESS, REDIS_PORT);
    for (int i = 0; i < MAX_DATA; i++) {
        dAlarm[i].id_tu = QString::number(qrand() % ((MAX_DATA*199 + 1) - 1000) + 1000);
        id_tu.append(dAlarm[i].id_tu);
        dAlarm[i].jml_rules = 0;

//        for (int j = 0; j < (qrand() % ((MAX_RULE + 1) - 1) + 1); j++) {
//        for (int j = 0; j < MAX_RULE; j++) {
        for (int j = 0; j < 1; j++) {
            dAlarm[i].rules[dAlarm[i].jml_rules].id_alarm = id++;
            if (j % 2 == 0) {
//                int val1 = qrand() % ((2000 + 1) - 1000) + 1000;
                int val1 = 0;
                dAlarm[i].rules[dAlarm[i].jml_rules].logic = ">";
                dAlarm[i].rules[dAlarm[i].jml_rules].value = QString::number(val1);
                dAlarm[i].rules[dAlarm[i].jml_rules].notif = "More_Than_"+ QString::number(val1);
            } else {
                int val2 = qrand() % ((1000 + 1) - 1) + 1;
                dAlarm[i].rules[dAlarm[i].jml_rules].logic = "<";
                dAlarm[i].rules[dAlarm[i].jml_rules].value = QString::number(val2);
                dAlarm[i].rules[dAlarm[i].jml_rules].notif = "More_Than_"+ QString::number(val2);
            }
            dAlarm[i].rules[dAlarm[i].jml_rules].noise_time = qrand() % ((5 + 1) - 1) + 1;
//            dAlarm[i].rules[dAlarm[i].jml_rules].noise_time = 1;
            dAlarm[i].rules[dAlarm[i].jml_rules].interval = qrand() % ((60 + 1) - 1) + 1;
//            dAlarm[i].rules[dAlarm[i].jml_rules].interval = 1;
            dAlarm[i].jml_rules++;
        }

        dAlarm[i].currentValue = 0;
        dAlarm[i].scan_period = qrand() % ((60 + 1) - 1) + 1;
//        dAlarm[i].scan_period = 1;
        dAlarm[i].next_execute = QDateTime::currentDateTime();

//        for (int j = 0; j < dAlarm[i].jml_rules; j++) {
//            rds.reqRedis("hset monita_alarm_service:test_rule " +
//                         dAlarm[i].id_tu + ";" + dAlarm[i].rules[j].logic + ";" + dAlarm[i].rules[j].value + " " +
//                         dAlarm[i].rules[j].notif
//                         , REDIS_ADDRESS, REDIS_PORT);
//        }

        jml_data_alarm++;
    }
//    rds.reqRedis("del monita_alarm_service:test_alarm", REDIS_ADDRESS, REDIS_PORT);

    source.doSetup(threadSource, id_tu, time_period, redis_address, redis_port);
    source.moveToThread(&threadSource);
    threadSource.start();

    db = db_mysql.connect_db("rules");
#else
    QStringList temp = cfg.read("CONFIG");
    time_period = temp.at(0).toInt();
    temp = cfg.read("REDIS");
    redis_address = temp.at(0);
    redis_port = temp.at(1).toInt();

    db = db_mysql.connect_db("rules");
    readAlarmParameter();
#endif

    notf = new notification();
    connect(this, SIGNAL(sendNotif(QStringList,QDateTime,int)), notf, SLOT(RedisToJson(QStringList,QDateTime,int)));
//    notf->sendMail(
//                "application.beta.tester@gmail.com",
//                "dendygema-P@$$w0rd",
//                "smtp.gmail.com",
//                465,
//                "dendy@daunbiru.com",
//                "Test Lagi yang ke empat !!",
//                "Yang ini beneran ..");
//    notf->doSetup(threadNotf);
//    notf->moveToThread(&threadNotf);
//    threadNotf.start();

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(time_period);
}

void worker::doWork()
{
    QDateTime temp_dateTime = QDateTime::currentDateTime();
    qDebug() << "<< START Periksa Alarm << " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
    readCurrentValue();
    QString mysql_command;
    QStringList alarm;
    for (int i = 0; i < jml_data_alarm; i++) {
        if (dAlarm[i].next_execute <= temp_dateTime) {
            dAlarm[i].status = QString::number(dAlarm[i].currentValue) +  ";NORMAL";
            dAlarm[i].last_execute = temp_dateTime;

            for (int j = 0; j < dAlarm[i].jml_rules; j++) {
                if (dAlarm[i].rules[j].logic == ">") {
                    QStringList temp_val = dAlarm[i].rules[j].value.split(";");
                    if (dAlarm[i].currentValue > temp_val.at(0).toInt()) {
                        this->processAlarm(i,j,alarm);
                        break;
                    } else {
                        dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs(dAlarm[i].scan_period);
                    }
                } else if (dAlarm[i].rules[j].logic == "<") {
                    QStringList temp_val = dAlarm[i].rules[j].value.split(";");
                    if (dAlarm[i].currentValue < temp_val.at(0).toInt()) {
                        this->processAlarm(i,j,alarm);
                        break;
                    } else {
                        dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs(dAlarm[i].scan_period);
                    }
                } else if (dAlarm[i].rules[j].logic == ">=") {
                    QStringList temp_val = dAlarm[i].rules[j].value.split(";");
                    if (dAlarm[i].currentValue >= temp_val.at(0).toInt()) {
                        this->processAlarm(i,j,alarm);
                        break;
                    } else {
                        dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs(dAlarm[i].scan_period);
                    }
                } else if (dAlarm[i].rules[j].logic == "<=") {
                    QStringList temp_val = dAlarm[i].rules[j].value.split(";");
                    if (dAlarm[i].currentValue <= temp_val.at(0).toInt()) {
                        this->processAlarm(i,j,alarm);
                        break;
                    } else {
                        dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs(dAlarm[i].scan_period);
                    }
                } else if (dAlarm[i].rules[j].logic == "=") {
                    QStringList temp_val = dAlarm[i].rules[j].value.split(";");
                    if (dAlarm[i].currentValue == temp_val.at(0).toInt()) {
                        this->processAlarm(i,j,alarm);
                        break;
                    } else {
                        dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs(dAlarm[i].scan_period);
                    }
                } else if (dAlarm[i].rules[j].logic == "BETWEEN") {
                    QStringList temp_val = dAlarm[i].rules[j].value.split(";");
                    if (dAlarm[i].currentValue < temp_val.at(0).toInt() && dAlarm[i].currentValue > temp_val.at(1).toInt()) {
                        this->processAlarm(i,j,alarm);
                        break;
                    } else {
                        dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs(dAlarm[i].scan_period);
                    }
                }
            }
        }
    }
    for (int i = 0; i < notf->m_clients.length(); i++) {
        emit sendNotif(alarm, temp_dateTime, i);
    }
    if (alarm.length() > 0) {
        for (int i = 0; i < alarm.length(); i+=5) {
            if (i == alarm.length()-5) {
                mysql_command = mysql_command +
                        "(\\'" + alarm.at(i) +              // ID Alarm
                        "\\',\\'" + alarm.at(i+1) +         // ID Titik Ukur
                        "\\',\\'" + alarm.at(i+2) +         // Last Execute
                        "\\',\\'" + alarm.at(i+3) +         // Current Value
                        "\\',\\'" + alarm.at(i+4) + "\\')"; // Status
            } else {
                mysql_command = mysql_command +
                        "(\\'" + alarm.at(i) +
                        "\\',\\'" + alarm.at(i+1) +
                        "\\',\\'" + alarm.at(i+2) +
                        "\\',\\'" + alarm.at(i+3) +
                        "\\',\\'" + alarm.at(i+4) + "\\'),";
            }
        }
        db_mysql.write_alarm_history(db, mysql_command, "Test", 0);
    }
    qDebug() << ">> *END* Periksa Alarm >> " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
}

void worker::processAlarm(int idx_alarm, int idx_rules, QStringList &alarm)
{
    if (dAlarm[idx_alarm].rules[idx_rules].temp_noise_time > 0) {
        dAlarm[idx_alarm].rules[idx_rules].temp_noise_time--;
        if (dAlarm[idx_alarm].rules[idx_rules].temp_noise_time == 0) {
            dAlarm[idx_alarm].status = dAlarm[idx_alarm].rules[idx_rules].notif;
            dAlarm[idx_alarm].last_execute = QDateTime::fromTime_t(dAlarm[idx_alarm].last_execute.toTime_t() - dAlarm[idx_alarm].rules[idx_rules].noise_time);
//            dAlarm[idx_alarm].next_execute = dAlarm[idx_alarm].last_execute.addSecs(dAlarm[idx_alarm].rules[idx_rules].interval);
            dAlarm[idx_alarm].next_execute = QDateTime::fromTime_t(dAlarm[idx_alarm].last_execute.toTime_t() + dAlarm[idx_alarm].rules[idx_rules].interval - dAlarm[idx_alarm].scan_period);

            alarm.append(QString::number(dAlarm[idx_alarm].rules[idx_rules].id_alarm));
            alarm.append(dAlarm[idx_alarm].id_tu);
//            QString temp = QString::number(dAlarm[idx_alarm].last_execute.toTime_t() - dAlarm[idx_alarm].rules[idx_rules].noise_time);
            alarm.append(QString::number(dAlarm[idx_alarm].last_execute.toTime_t()));
            alarm.append(QString::number(dAlarm[idx_alarm].currentValue));
            alarm.append(dAlarm[idx_alarm].status);
        } else {
            dAlarm[idx_alarm].next_execute = dAlarm[idx_alarm].last_execute.addSecs(1);
        }
    } else {
        dAlarm[idx_alarm].rules[idx_rules].temp_noise_time = dAlarm[idx_alarm].rules[idx_rules].noise_time;
    }
}

void worker::readCurrentValue()
{
#ifdef SIMULATION
    QStringList request = rds.reqRedis("hlen monita_alarm_service:test_data", redis_address, redis_port);
#else
    QStringList request = rds.reqRedis("hlen monita_service:realtime", redis_address, redis_port);
#endif
    if (request.length() > 0) {
        int redis_len = request.at(0).toInt();
#ifdef SIMULATION
        request = rds.reqRedis("hgetall monita_alarm_service:test_data", redis_address, redis_port, redis_len*2);
#else
        request = rds.reqRedis("hgetall monita_service:realtime", redis_address, redis_port, redis_len*2);
#endif
        for (int i = 0; i < request.length(); i+=2) {
            for (int j = 0; j < jml_data_alarm; j++) {
                if (request.at(i) == dAlarm[j].id_tu) {
                    dAlarm[j].currentValue = request.at(i+1).toInt();
                    break;
                }
            }
        }
    }
}

void worker::readAlarmParameter()
{
//    db_mysql.read_titik_ukur(db, 1, "Test", 0);
    QStringList dataAlarm = db_mysql.read_data_alarm(db, "Test", 0);
    QTime time_temp;
    jml_data_alarm = 0;
    if (dataAlarm.length() > 0) {
        for (int i = 0; i < dataAlarm.length(); i+=9) {
            if (jml_data_alarm == 0) {
                dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].id_alarm = dataAlarm.at(i).toInt();
                dAlarm[jml_data_alarm].id_tu = dataAlarm.at(i+1);
                dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].logic = dataAlarm.at(i+2);
                dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].notif = dataAlarm.at(i+3);
                dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].value = dataAlarm.at(i+4);
                dAlarm[jml_data_alarm]
                        .rules[dAlarm[jml_data_alarm]
                        .jml_rules].value = dAlarm[jml_data_alarm]
                                            .rules[dAlarm[jml_data_alarm]
                                            .jml_rules].value +
                        ";" + dataAlarm.at(i+5);
                time_temp = QTime::fromString(dataAlarm.at(i+6), "HH:mm:ss");
                dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].noise_time = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                time_temp = QTime::fromString(dataAlarm.at(i+7), "HH:mm:ss");
                dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].interval = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                time_temp = QTime::fromString(dataAlarm.at(i+8), "HH:mm:ss");
                dAlarm[jml_data_alarm].scan_period = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                dAlarm[jml_data_alarm].jml_rules++;
                jml_data_alarm++;

//                id_tu.append(dataAlarm.at(i+1));
            } else {
                if (dAlarm[jml_data_alarm-1].id_tu == dataAlarm.at(i+1)) {
                    dAlarm[jml_data_alarm-1].rules[dAlarm[jml_data_alarm-1].jml_rules].id_alarm = dataAlarm.at(i).toInt();
                    dAlarm[jml_data_alarm-1].id_tu = dataAlarm.at(i+1);
                    dAlarm[jml_data_alarm-1].rules[dAlarm[jml_data_alarm-1].jml_rules].logic = dataAlarm.at(i+2);
                    dAlarm[jml_data_alarm-1].rules[dAlarm[jml_data_alarm-1].jml_rules].notif = dataAlarm.at(i+3);
                    dAlarm[jml_data_alarm-1].rules[dAlarm[jml_data_alarm-1].jml_rules].value = dataAlarm.at(i+4);
                    dAlarm[jml_data_alarm-1]
                            .rules[dAlarm[jml_data_alarm-1]
                            .jml_rules].value = dAlarm[jml_data_alarm-1]
                                                .rules[dAlarm[jml_data_alarm-1]
                                                .jml_rules].value +
                            ";" + dataAlarm.at(i+5);
                    time_temp = QTime::fromString(dataAlarm.at(i+6), "HH:mm:ss");
                    dAlarm[jml_data_alarm-1].rules[dAlarm[jml_data_alarm-1].jml_rules].noise_time = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    time_temp = QTime::fromString(dataAlarm.at(i+7), "HH:mm:ss");
                    dAlarm[jml_data_alarm-1].rules[dAlarm[jml_data_alarm-1].jml_rules].interval = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    time_temp = QTime::fromString(dataAlarm.at(i+8), "HH:mm:ss");
                    dAlarm[jml_data_alarm-1].scan_period = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    dAlarm[jml_data_alarm-1].jml_rules++;
                } else {
                    dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].id_alarm = dataAlarm.at(i).toInt();
                    dAlarm[jml_data_alarm].id_tu = dataAlarm.at(i+1);
                    dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].logic = dataAlarm.at(i+2);
                    dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].notif = dataAlarm.at(i+3);
                    dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].value = dataAlarm.at(i+4);
                    dAlarm[jml_data_alarm]
                            .rules[dAlarm[jml_data_alarm]
                            .jml_rules].value = dAlarm[jml_data_alarm]
                                                .rules[dAlarm[jml_data_alarm]
                                                .jml_rules].value +
                            ";" + dataAlarm.at(i+5);
                    time_temp = QTime::fromString(dataAlarm.at(i+6), "HH:mm:ss");
                    dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].noise_time = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    time_temp = QTime::fromString(dataAlarm.at(i+7), "HH:mm:ss");
                    dAlarm[jml_data_alarm].rules[dAlarm[jml_data_alarm].jml_rules].interval = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    time_temp = QTime::fromString(dataAlarm.at(i+8), "HH:mm:ss");
                    dAlarm[jml_data_alarm].scan_period = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    dAlarm[jml_data_alarm].jml_rules++;
                    jml_data_alarm++;

//                    id_tu.append(dataAlarm.at(i+1));
                }
            }
        }
    } else {
        return;
    }
}
