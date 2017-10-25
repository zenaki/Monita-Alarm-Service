#include "datasource.h"
#include "worker.h"

dataSource::dataSource(QObject *parent) : QObject(parent)
{

}

void dataSource::doSetup(QThread &cThread, QStringList titik_ukur)
{
    id_tu = titik_ukur;
    connect(&cThread, SIGNAL(started()), this, SLOT(doWork()));

    rds.reqRedis("del monita_alarm_service:test_data"
                 , REDIS_ADDRESS, REDIS_PORT);

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(TIME_PERIOD);
}

void dataSource::doWork()
{
    qDebug() << "<< START Update Value << " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
    for (int i = 0; i < id_tu.length(); i++) {
        rds.reqRedis("hset monita_alarm_service:test_data " +
                     id_tu.at(i) + " " +
                     QString::number(qrand() % ((2000 + 1) - 1) + 1)
                     , REDIS_ADDRESS, REDIS_PORT);
    }
    qDebug() << ">> *END* Update Value >> " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
}
