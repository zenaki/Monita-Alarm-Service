#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QThread>

#include "utama.h"

class dataSource : public QObject
{
    Q_OBJECT
public:
    explicit dataSource(QObject *parent = 0);

    void doSetup(QThread &cThread, QStringList titik_ukur, int t_period, QString r_address, int r_port);

private:
    redis rds;
    QStringList id_tu;

    int time_period;
    QString redis_address;
    int redis_port;

private slots:
    void doWork();
};

#endif // DATASOURCE_H
