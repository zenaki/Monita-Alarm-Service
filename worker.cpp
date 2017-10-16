#include "worker.h"

worker::worker(QObject *parent) : QObject(parent)
{
    QStringList id_tu;
    rds.reqRedis("del monita_alarm_service:test_rule", REDIS_ADDRESS, REDIS_PORT);
    for (int i = 0; i < MAX_DATA; i++) {
        dAlarm[i].id_tu = QString::number(qrand() % ((5000 + 1) - 1000) + 1000);
        id_tu.append(dAlarm[i].id_tu);
        dAlarm[i].jml_rules = 0;

//        for (int j = 0; j < (qrand() % ((MAX_RULE + 1) - 1) + 1); j++) {
        for (int j = 0; j < MAX_RULE; j++) {
            if (j % 2 == 0) {
                int val1 = qrand() % ((2000 + 1) - 1000) + 1000;
                dAlarm[i].rules[dAlarm[i].jml_rules].logic = "mt";
                dAlarm[i].rules[dAlarm[i].jml_rules].value = QString::number(val1);
                dAlarm[i].rules[dAlarm[i].jml_rules].notif = "More_Than_"+ QString::number(val1);
            } else {
                int val2 = qrand() % ((1000 + 1) - 1) + 1;
                dAlarm[i].rules[dAlarm[i].jml_rules].logic = "lt";
                dAlarm[i].rules[dAlarm[i].jml_rules].value = QString::number(val2);
                dAlarm[i].rules[dAlarm[i].jml_rules].notif = "More_Than_"+ QString::number(val2);
            }
//            dAlarm[i].rules[dAlarm[i].jml_rules].noise_time = qrand() % ((5 + 1) - 1) + 1;
            dAlarm[i].rules[dAlarm[i].jml_rules].noise_time = 3;
//            dAlarm[i].rules[dAlarm[i].jml_rules].interval = qrand() % ((60 + 1) - 1) + 1;
            dAlarm[i].rules[dAlarm[i].jml_rules].interval = 10;
            dAlarm[i].jml_rules++;
        }

        dAlarm[i].currentValue = 0;
//        dAlarm[i].scan_period = qrand() % ((60 + 1) - 1) + 1;
        dAlarm[i].scan_period = 1;
        dAlarm[i].next_execute = QDateTime::currentDateTime();

        for (int j = 0; j < dAlarm[i].jml_rules; j++) {
            rds.reqRedis("hset monita_alarm_service:test_rule " +
                         dAlarm[i].id_tu + ";" + dAlarm[i].rules[j].logic + ";" + dAlarm[i].rules[j].value + " " +
                         dAlarm[i].rules[j].notif
                         , REDIS_ADDRESS, REDIS_PORT);
        }
    }
    rds.reqRedis("del monita_alarm_service:test_alarm", REDIS_ADDRESS, REDIS_PORT);

    source.doSetup(threadSource, id_tu);
    source.moveToThread(&threadSource);
    threadSource.start();

    notf = new notification();
    connect(this, SIGNAL(sendNotif(QStringList,QDateTime,int)), notf, SLOT(RedisToJson(QStringList,QDateTime,int)));
//    notf->doSetup(threadNotf);
//    notf->moveToThread(&threadNotf);
//    threadNotf.start();

//    db_mysql.connect_db("rules");

//    int test;
//    for (int i = 0; i < 50000; i++) {
//        test = i;
//    }

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(TIME_PERIOD);
}

void worker::doWork()
{
    qDebug() << "<< START Periksa Alarm << " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
    readCurrentValue();
    QString redis_command;
    QStringList alarm;
    for (int i = 0; i < MAX_DATA; i++) {
        if (dAlarm[i].next_execute <= QDateTime::currentDateTime()) {
            dAlarm[i].status = QString::number(dAlarm[i].currentValue) +  ";NORMAL";
            dAlarm[i].last_execute = QDateTime::currentDateTime();

            for (int j = 0; j < dAlarm[i].jml_rules; j++) {
                if (dAlarm[i].rules[j].logic == "mt") {
                    if (dAlarm[i].currentValue > dAlarm[i].rules[j].value.toInt()) {
                        if (dAlarm[i].rules[j].temp_noise_time > 0) {
                            dAlarm[i].rules[j].temp_noise_time--;
                            if (dAlarm[i].rules[j].temp_noise_time == 0) {
                                dAlarm[i].status = QString::number(dAlarm[i].currentValue) + ";" + dAlarm[i].rules[j].notif;
                                dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs((dAlarm[i].rules[j].noise_time * (-1)) + dAlarm[i].rules[j].interval);
                            } else {
                                dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs(1);
                            }
                        } else {
                            dAlarm[i].rules[j].temp_noise_time = dAlarm[i].rules[j].noise_time;
                        }
                        break;
                    }
                } else if (dAlarm[i].rules[j].logic == "lt") {
                    if (dAlarm[i].currentValue < dAlarm[i].rules[j].value.toInt()) {
                        if (dAlarm[i].rules[j].temp_noise_time > 0) {
                            dAlarm[i].rules[j].temp_noise_time--;
                            if (dAlarm[i].rules[j].temp_noise_time == 0) {
                                dAlarm[i].status = QString::number(dAlarm[i].currentValue) + ";" + dAlarm[i].rules[j].notif;
                                dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs((dAlarm[i].rules[j].noise_time * (-1)) + dAlarm[i].rules[j].interval);
                            } else {
                                dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs(1);
                            }
                        } else {
                            dAlarm[i].rules[j].temp_noise_time = dAlarm[i].rules[j].noise_time;
                        }
                        break;
                    }
                } else {
                    dAlarm[i].next_execute = dAlarm[i].last_execute.addSecs(dAlarm[i].scan_period);
                }
            }

//            if (redis_command.isEmpty()) {
//                redis_command.append("hmset monita_alarm_service:test_alarm " +
//                                     dAlarm[i].id_tu + " " +
//                                     dAlarm[i].status + ";" +
//                                     dAlarm[i].last_execute.toString("yyyy-MM-dd_HH:mm:ss:zzz"));
//            } else {
//                redis_command.append(" " +
//                                     dAlarm[i].id_tu + " " +
//                                     dAlarm[i].status + ";" +
//                                     dAlarm[i].last_execute.toString("yyyy-MM-dd_HH:mm:ss:zzz"));
//            }

            alarm.append(dAlarm[i].id_tu);
            alarm.append(dAlarm[i].status + ";" +
                         dAlarm[i].last_execute.toString("yyyy-MM-dd_HH:mm:ss:zzz"));
        }
    }
//    if (!redis_command.isEmpty()) {
////        threadNotf.wait();
//        rds.reqRedis(redis_command, REDIS_ADDRESS, REDIS_PORT);
//    }
    for (int i = 0; i < notf->m_clients.length(); i++) {
        emit sendNotif(alarm, QDateTime::currentDateTime(), i);
    }
    qDebug() << ">> *END* Periksa Alarm >> " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
}

void worker::readCurrentValue() {
    QStringList request = rds.reqRedis("hlen monita_alarm_service:test_data", REDIS_ADDRESS, REDIS_PORT);
    if (request.length() > 0) {
        int redis_len = request.at(0).toInt();
        request = rds.reqRedis("hgetall monita_alarm_service:test_data", REDIS_ADDRESS, REDIS_PORT, redis_len*2);
        for (int i = 0; i < request.length(); i+=2) {
            for (int j = 0; j < MAX_DATA; j++) {
                if (request.at(i) == dAlarm[j].id_tu) {
                    dAlarm[j].currentValue = request.at(i+1).toInt();
                    break;
                }
            }
        }
    }
}
