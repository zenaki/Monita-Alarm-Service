#include "datasource.h"
#include "worker.h"

dataSource::dataSource(QObject *parent) : QObject(parent)
{

}

void dataSource::doSetup(QThread &cThread, QStringList titik_ukur, int t_period, QString r_address, int r_port)
{   
    time_period = t_period;
    redis_address = r_address;
    redis_port = r_port;

    id_tu = titik_ukur;
    connect(&cThread, SIGNAL(started()), this, SLOT(doWork()));

    rds.reqRedis("del monita_alarm_service:test_data"
                 , redis_address, redis_port);

    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(doWork()));
    t->start(time_period);
}

void dataSource::doWork()
{
    qDebug() << "<< START Update Value << " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
    for (int i = 0; i < id_tu.length(); i++) {
        rds.reqRedis("hset monita_alarm_service:test_data " +
                     id_tu.at(i) + " " +
//                     QString::number(qrand() % ((2000 + 1) - 1) + 1)
                     QString::number(10)
                     , redis_address, redis_port);
    }
    qDebug() << ">> *END* Update Value >> " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz");
}
