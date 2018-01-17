#include "worker.h"

worker::worker(QObject *parent) : QObject(parent)
{   
//    QStringList id_tu;
//    int id = 0;
////    rds.reqRedis("del monita_alarm_service:test_rule", REDIS_ADDRESS, REDIS_PORT);
//    for (int i = 0; i < MAX_DATA; i++) {
//        dAlarm[i].id_tu = QString::number(qrand() % ((5000 + 1) - 1000) + 1000);
//        id_tu.append(dAlarm[i].id_tu);
//        dAlarm[i].jml_rules = 0;

////        for (int j = 0; j < (qrand() % ((MAX_RULE + 1) - 1) + 1); j++) {
//        for (int j = 0; j < MAX_RULE; j++) {
//            dAlarm[i].rules[dAlarm[i].jml_rules].id_alarm = id++;
//            if (j % 2 == 0) {
//                int val1 = qrand() % ((2000 + 1) - 1000) + 1000;
//                dAlarm[i].rules[dAlarm[i].jml_rules].logic = "mt";
//                dAlarm[i].rules[dAlarm[i].jml_rules].value = QString::number(val1);
//                dAlarm[i].rules[dAlarm[i].jml_rules].notif = "More_Than_"+ QString::number(val1);
//            } else {
//                int val2 = qrand() % ((1000 + 1) - 1) + 1;
//                dAlarm[i].rules[dAlarm[i].jml_rules].logic = "lt";
//                dAlarm[i].rules[dAlarm[i].jml_rules].value = QString::number(val2);
//                dAlarm[i].rules[dAlarm[i].jml_rules].notif = "More_Than_"+ QString::number(val2);
//            }
////            dAlarm[i].rules[dAlarm[i].jml_rules].noise_time = qrand() % ((5 + 1) - 1) + 1;
//            dAlarm[i].rules[dAlarm[i].jml_rules].noise_time = 1;
////            dAlarm[i].rules[dAlarm[i].jml_rules].interval = qrand() % ((60 + 1) - 1) + 1;
//            dAlarm[i].rules[dAlarm[i].jml_rules].interval = 1;
//            dAlarm[i].jml_rules++;
//        }

//        dAlarm[i].currentValue = 0;
////        dAlarm[i].scan_period = qrand() % ((60 + 1) - 1) + 1;
//        dAlarm[i].scan_period = 1;
//        dAlarm[i].next_execute = QDateTime::currentDateTime();

////        for (int j = 0; j < dAlarm[i].jml_rules; j++) {
////            rds.reqRedis("hset monita_alarm_service:test_rule " +
////                         dAlarm[i].id_tu + ";" + dAlarm[i].rules[j].logic + ";" + dAlarm[i].rules[j].value + " " +
////                         dAlarm[i].rules[j].notif
////                         , REDIS_ADDRESS, REDIS_PORT);
////        }
//    }
//    rds.reqRedis("del monita_alarm_service:test_alarm", REDIS_ADDRESS, REDIS_PORT);
    QStringList temp = cfg.read("CONFIG");
    time_period = temp.at(0).toInt();

    temp = cfg.read("REDIS");
    redis_address = temp.at(0);
    redis_port = temp.at(1).toInt();

    temp = cfg.read("EMAIL");
    email_sender = temp.at(0);
    email_password = temp.at(1);
    email_server = temp.at(2);
    email_port = temp.at(3).toInt();
    email_recipient = temp.at(4);

    db = db_mysql.connect_db("rules");
    readAlarmParameter();

//    source.doSetup(threadSource, id_tu);
//    source.moveToThread(&threadSource);
//    threadSource.start();

    rds.reqRedis("del monita_alarm_service:notification", redis_address, redis_port);
    notf = new notification();
    connect(this, SIGNAL(sendNotif(QStringList,QDateTime,int)), notf, SLOT(RedisToJson(QStringList,QDateTime,int)));
    notf->doSetup(threadNotf);
    notf->moveToThread(&threadNotf);
    threadNotf.start();

    schd = new scheduler(0, notf);
    schd->doSetup(threadSchd);
    schd->moveToThread(&threadSchd);
    threadSchd.start();


////    QString expression_string("3 + Math.sqrt(25) + Math.pow(3,2) + log(5)");
//////    QString expression_string("Math.log(1000)");
//////    QString expression_string("(1/2)+(3/4)");
//    QScriptEngine expression;
////    double my_val = expression.evaluate(expression_string).toNumber();

//    QScriptValue fun = expression.evaluate("(function(a, b) { "
//                                           "    if (a < b) { "
//                                           "        return a; "
//                                           "    } else { "
//                                           "        return b; "
//                                           "    } "
//                                           " }) ");
//    QScriptValueList args;
//    args << 3 << 2;
////    a{*}
//    double my_val = fun.call(QScriptValue(), args).toNumber();

    log.write("Email", "Sending Email for the first time ..........", 0);
//    qDebug() << "Send Email ..........";
//    notf->sendMail(
//                "application.beta.tester@gmail.com",
//                "dendygema-P@$$w0rd",
//                "smtp.gmail.com",
//                465,
//                "dendy@daunbiru.com",
//                "OVM - Server",
//                "Test Kirim Email Attachment 15",
//                QStringList("test.pdf"));

    notf->sendMail(
                email_sender,
                email_password,
                email_server,
                email_port,
                email_recipient,
                "Monita Alarm Service",
                "Service baru saja direstart ..",
                QStringList("/home/ovm/test.pdf"));

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(time_period);
    doWork();
//    qDebug() << "Test";
}

void worker::doWork()
{
    QDateTime temp_dateTime = QDateTime::currentDateTime();
    log.write("ALARM", "START Periksa Alarm", 0);
//    qDebug() << "<< START Periksa Alarm << " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
    readCurrentValue();
//    QString mysql_command;
    QStringList alarm;
    for (int i = 0; i < jml_data_alarm; i++) {
        dAlarm = DataAlarm.at(i);
        if (dAlarm.next_execute <= temp_dateTime) {
            dAlarm.status = QString::number(dAlarm.currentValue) +  ";NORMAL";
            dAlarm.last_execute = temp_dateTime;

            for (int j = 0; j < dAlarm.jml_rules; j++) {
                if (dAlarm.rules[j].logic == ">") {
                    QStringList temp_val = dAlarm.rules[j].value.split(";");
                    if (dAlarm.currentValue > temp_val.at(0).toInt()) {
                        this->processAlarm(j,alarm);
                        break;
                    } else {
                        dAlarm.next_execute = dAlarm.last_execute.addSecs(dAlarm.scan_period);
                    }
                } else if (dAlarm.rules[j].logic == "<") {
                    QStringList temp_val = dAlarm.rules[j].value.split(";");
//                    if (dAlarm.id_tu == "848019") {
//                        qDebug() << "";
//                    }
                    if (dAlarm.currentValue < temp_val.at(0).toInt()) {
                        this->processAlarm(j,alarm);
                        break;
                    } else {
                        dAlarm.next_execute = dAlarm.last_execute.addSecs(dAlarm.scan_period);
                    }
                } else if (dAlarm.rules[j].logic == ">=") {
                    QStringList temp_val = dAlarm.rules[j].value.split(";");
                    if (dAlarm.currentValue >= temp_val.at(0).toInt()) {
                        this->processAlarm(j,alarm);
                        break;
                    } else {
                        dAlarm.next_execute = dAlarm.last_execute.addSecs(dAlarm.scan_period);
                    }
                } else if (dAlarm.rules[j].logic == "<=") {
                    QStringList temp_val = dAlarm.rules[j].value.split(";");
                    if (dAlarm.currentValue <= temp_val.at(0).toInt()) {
                        this->processAlarm(j,alarm);
                        break;
                    } else {
                        dAlarm.next_execute = dAlarm.last_execute.addSecs(dAlarm.scan_period);
                    }
                } else if (dAlarm.rules[j].logic == "=") {
                    QStringList temp_val = dAlarm.rules[j].value.split(";");
                    if (dAlarm.currentValue == temp_val.at(0).toInt()) {
                        this->processAlarm(j,alarm);
                        break;
                    } else {
                        dAlarm.next_execute = dAlarm.last_execute.addSecs(dAlarm.scan_period);
                    }
                } else if (dAlarm.rules[j].logic == "BETWEEN") {
                    QStringList temp_val = dAlarm.rules[j].value.split(";");
                    if (dAlarm.currentValue < temp_val.at(0).toInt() && dAlarm.currentValue > temp_val.at(1).toInt()) {
                        this->processAlarm(j,alarm);
                        break;
                    } else {
                        dAlarm.next_execute = dAlarm.last_execute.addSecs(dAlarm.scan_period);
                    }
                }
            }
        }
        DataAlarm[i] = dAlarm;
    }
    for (int i = 0; i < alarm.length(); i+=5) {
        rds.reqRedis("hset monita_alarm_service:notification " +
                     alarm.at(i) + ";" + alarm.at(i+1) + ";" + alarm.at(i+2) + " " +
                     alarm.at(i+3) + ";" + alarm.at(i+4)
                     , redis_address, redis_port);
    }
    log.write("ALARM", "END Periksa Alarm", 0);
//    qDebug() << ">> *END* Periksa Alarm >> " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");

//    for (int i = 0; i < jml_data_alarm; i++) {
//        ProcessAlarm *procAlarm = new ProcessAlarm;
//        QThread *threadAlarm = new QThread;

//        dAlarm = DataAlarm.at(i);
//        procAlarm->doSetup(threadAlarm, dAlarm);
//        procAlarm->moveToThread(threadAlarm);

//        AlarmObj << procAlarm;
//        AlarmThread << threadAlarm;

//        AlarmThread[i]->start();
//    }
}

void worker::processAlarm(int idx_rules, QStringList &alarm)
{
    if (dAlarm.rules[idx_rules].temp_noise_time > 0) {
        dAlarm.rules[idx_rules].temp_noise_time--;
        if (dAlarm.rules[idx_rules].temp_noise_time == 0) {
            dAlarm.status = dAlarm.rules[idx_rules].notif;
            dAlarm.last_execute = QDateTime::fromTime_t(dAlarm.last_execute.toTime_t() - dAlarm.rules[idx_rules].noise_time);
            if (dAlarm.rules[idx_rules].interval > dAlarm.scan_period) {
//                dAlarm.next_execute = QDateTime::fromTime_t(
//                            dAlarm.last_execute.toTime_t() + dAlarm.rules[idx_rules].interval - dAlarm.scan_period);
                dAlarm.next_execute = dAlarm.last_execute.addSecs(dAlarm.rules[idx_rules].interval);
            } else {
                dAlarm.next_execute = dAlarm.last_execute.addSecs(dAlarm.rules[idx_rules].interval);
            }
//            if (dAlarm.id_tu == "848019") {
//                qDebug() << "";
//            }
            alarm.append(QString::number(dAlarm.rules[idx_rules].id_alarm));
            alarm.append(dAlarm.id_tu);
//            QString temp = QString::number(dAlarm.last_execute.toTime_t() - dAlarm.rules[idx_rules].noise_time);
            alarm.append(QString::number(dAlarm.last_execute.toMSecsSinceEpoch())
                         .left(QString::number(dAlarm.last_execute.toMSecsSinceEpoch()).length() - 3));
            alarm.append(QString::number(dAlarm.currentValue));
            alarm.append(dAlarm.status.replace(" ", "_"));
        } else {
            dAlarm.next_execute = dAlarm.last_execute.addMSecs(time_period/2);
        }
    } else {
        dAlarm.rules[idx_rules].temp_noise_time = dAlarm.rules[idx_rules].noise_time;
    }
}

void worker::readCurrentValue()
{
//    QStringList request = rds.reqRedis("hlen monita_alarm_service:test_data", REDIS_ADDRESS, REDIS_PORT);
    QStringList request = rds.reqRedis("hlen monita_service:realtime", redis_address, redis_port);
    if (request.length() > 0) {
        int redis_len = request.at(0).toInt();
//        request = rds.reqRedis("hgetall monita_alarm_service:test_data", REDIS_ADDRESS, REDIS_PORT, redis_len*2);
        QStringList request = rds.reqRedis("hgetall monita_service:realtime", redis_address, redis_port, redis_len*2);
        for (int i = 0; i < request.length(); i+=2) {
            for (int j = 0; j < jml_data_alarm; j++) {
                dAlarm = DataAlarm.at(j);
                QString titik_ukur = request.at(i).split(";").at(1);
                if (titik_ukur == dAlarm.id_tu) {
                    QString currVal = request.at(i+1).split(";").at(1);
                    dAlarm.currentValue = currVal.toDouble();
                    DataAlarm[j] = dAlarm;
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
//    while (dataAlarm.length() == 0) {
//        dataAlarm = db_mysql.read_data_alarm(db, "Test", 0);
//    }
    QTime time_temp;
    jml_data_alarm = 0;
    if (dataAlarm.length() > 0) {
        for (int i = 0; i < dataAlarm.length(); i+=9) {
            if (jml_data_alarm == 0) {
                struct data_alarm temp_data;
                dAlarm = temp_data;
                dAlarm.rules[dAlarm.jml_rules].id_alarm = dataAlarm.at(i).toInt();
                dAlarm.id_tu = dataAlarm.at(i+1);
                dAlarm.rules[dAlarm.jml_rules].logic = dataAlarm.at(i+2);
                dAlarm.rules[dAlarm.jml_rules].notif = dataAlarm.at(i+3);
                dAlarm.rules[dAlarm.jml_rules].value = dataAlarm.at(i+4);
                dAlarm.rules[dAlarm.jml_rules].value = dAlarm.rules[dAlarm.jml_rules].value + ";" + dataAlarm.at(i+5);
                time_temp = QTime::fromString(dataAlarm.at(i+6), "HH:mm:ss");
                dAlarm.rules[dAlarm.jml_rules].noise_time = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                time_temp = QTime::fromString(dataAlarm.at(i+7), "HH:mm:ss");
                dAlarm.rules[dAlarm.jml_rules].interval = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                time_temp = QTime::fromString(dataAlarm.at(i+8), "HH:mm:ss");
                dAlarm.scan_period = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                dAlarm.jml_rules++;
                jml_data_alarm++;

//                id_tu.append(dataAlarm.at(i+1));
                DataAlarm << dAlarm;
            } else {
                dAlarm = DataAlarm.at(DataAlarm.length()-1);
                if (dAlarm.id_tu == dataAlarm.at(i+1)) {
                    dAlarm.rules[dAlarm.jml_rules].id_alarm = dataAlarm.at(i).toInt();
                    dAlarm.id_tu = dataAlarm.at(i+1);
                    dAlarm.rules[dAlarm.jml_rules].logic = dataAlarm.at(i+2);
                    dAlarm.rules[dAlarm.jml_rules].notif = dataAlarm.at(i+3);
                    dAlarm.rules[dAlarm.jml_rules].value = dataAlarm.at(i+4);
                    dAlarm.rules[dAlarm.jml_rules].value = dAlarm.rules[dAlarm.jml_rules].value + ";" + dataAlarm.at(i+5);
                    time_temp = QTime::fromString(dataAlarm.at(i+6), "HH:mm:ss");
                    dAlarm.rules[dAlarm.jml_rules].noise_time = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    time_temp = QTime::fromString(dataAlarm.at(i+7), "HH:mm:ss");
                    dAlarm.rules[dAlarm.jml_rules].interval = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    time_temp = QTime::fromString(dataAlarm.at(i+8), "HH:mm:ss");
                    dAlarm.scan_period = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    dAlarm.jml_rules++;

                    DataAlarm[DataAlarm.length()-1] = dAlarm;
                } else {
                    struct data_alarm temp_data;
                    dAlarm = temp_data;
                    dAlarm.rules[dAlarm.jml_rules].id_alarm = dataAlarm.at(i).toInt();
                    dAlarm.id_tu = dataAlarm.at(i+1);
                    dAlarm.rules[dAlarm.jml_rules].logic = dataAlarm.at(i+2);
                    dAlarm.rules[dAlarm.jml_rules].notif = dataAlarm.at(i+3);
                    dAlarm.rules[dAlarm.jml_rules].value = dataAlarm.at(i+4);
                    dAlarm.rules[dAlarm.jml_rules].value = dAlarm.rules[dAlarm.jml_rules].value + ";" + dataAlarm.at(i+5);
                    time_temp = QTime::fromString(dataAlarm.at(i+6), "HH:mm:ss");
                    dAlarm.rules[dAlarm.jml_rules].noise_time = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    time_temp = QTime::fromString(dataAlarm.at(i+7), "HH:mm:ss");
                    dAlarm.rules[dAlarm.jml_rules].interval = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    time_temp = QTime::fromString(dataAlarm.at(i+8), "HH:mm:ss");
                    dAlarm.scan_period = time_temp.hour() * 3600 + time_temp.minute() * 60 + time_temp.second();
                    dAlarm.jml_rules++;
                    jml_data_alarm++;

//                    id_tu.append(dataAlarm.at(i+1));
                    DataAlarm << dAlarm;
                }
            }
        }
    } else {
        return;
    }
}
