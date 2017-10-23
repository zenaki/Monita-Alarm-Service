#ifndef UTAMA_H
#define UTAMA_H


#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QFile>
#include <QSettings>
#include <QDateTime>

#include <QDebug>
#include <QThread>
#include <QTimer>

#include "redis.h"

#define PATH_CONFIGURATION_JSON     ".MonAlaSerConfig/monita-alarm-cfg.json"
#define PATH_CONFIGURATION_BIN      ".MonAlaSerConfig/monita-alarm-cfg.bin"
#define PATH_DB_CONNECTION          ".MonAlaSerConfig/monita-alarm-db.dbe"
#define PATH_MONITA_LOG             ".MonAlaSerConfig/monita-alarm-log.txt"
#define PATH_MONITA                 ".MonAlaSerConfig"

//#define REDIS_ADDRESS   "192.168.3.4"
//#define REDIS_ADDRESS   "127.0.0.1"
//#define REDIS_PORT      6379

//#define TIME_PERIOD     1000

#define MAX_DATA        50
#define MAX_RULE        10

struct data_rules {
    int id_alarm;
    QString logic;
    QString value;
    QString notif;
    int noise_time;
    int temp_noise_time;
    int interval;
};

struct data_alarm {
    QString id_tu;
    int jml_rules;
    struct data_rules rules[2*MAX_RULE];
    int currentValue;
    QString status;

    int scan_period;
    QDateTime last_execute;
    QDateTime next_execute;
    QDateTime temp_time;
};

#endif // UTAMA_H
