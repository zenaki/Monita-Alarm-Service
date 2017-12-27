#include "processalarm.h"

ProcessAlarm::ProcessAlarm(QObject *parent) : QObject(parent)
{
    QStringList temp = cfg.read("CONFIG");
    time_period = temp.at(0).toInt();
    temp = cfg.read("REDIS");
    redis_address = temp.at(0);
    redis_port = temp.at(1).toInt();
}

void ProcessAlarm::doSetup(QThread *cThread, struct data_alarm DataAlarm)
{
    connect(cThread, SIGNAL(started()), this, SLOT(doWork()));
    dAlarm = DataAlarm;

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(time_period);
}

void ProcessAlarm::doWork()
{
    QDateTime temp_dateTime = QDateTime::currentDateTime();
    readCurrentValue();
    QStringList alarm;
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
    for (int i = 0; i < alarm.length(); i+=5) {
        rds.reqRedis("hset monita_alarm_service:notification " +
                     alarm.at(i) + ";" + alarm.at(i+1) + ";" + alarm.at(i+2) + " " +
                     alarm.at(i+3) + ";" + alarm.at(i+4)
                     , redis_address, redis_port);
    }
}

void ProcessAlarm::processAlarm(int idx_rules, QStringList &alarm)
{
    if (dAlarm.rules[idx_rules].temp_noise_time > 0) {
        dAlarm.rules[idx_rules].temp_noise_time--;
        if (dAlarm.rules[idx_rules].temp_noise_time == 0) {
            dAlarm.status = dAlarm.rules[idx_rules].notif;
            dAlarm.last_execute = QDateTime::fromTime_t(dAlarm.last_execute.toTime_t() - dAlarm.rules[idx_rules].noise_time);
            if (dAlarm.rules[idx_rules].interval > dAlarm.scan_period) {
                dAlarm.next_execute = QDateTime::fromTime_t(
                            dAlarm.last_execute.toTime_t() + dAlarm.rules[idx_rules].interval - dAlarm.scan_period);
            } else {
                dAlarm.next_execute = dAlarm.last_execute.addSecs(dAlarm.rules[idx_rules].interval);
            }
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

void ProcessAlarm::readCurrentValue()
{
    QStringList request = rds.reqRedis("hlen monita_service:realtime", redis_address, redis_port);
    if (request.length() > 0) {
        int redis_len = request.at(0).toInt();
        QStringList request = rds.reqRedis("hgetall monita_service:realtime", redis_address, redis_port, redis_len*2);
        for (int i = 0; i < request.length(); i+=2) {
            if (request.at(i) == dAlarm.id_tu) {
                dAlarm.currentValue = request.at(i+1).toInt();
                break;
            }
        }
    }
}
