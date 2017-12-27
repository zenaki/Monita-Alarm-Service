#include "scheduler.h"

scheduler::scheduler(QObject *parent) : QObject(parent)
{

}

scheduler::~scheduler()
{

}

void scheduler::doSetup(QThread &cThread)
{
    connect(&cThread, SIGNAL(started()), this, SLOT(doWork()));
}

void scheduler::doWork()
{
//    QCron cron("* 8-12,14-17 * * 1-5 *");
//    cron = new QCron("* * * * * *");
    cron = new QCron("* 13 * * * *");
//    QCron cron("* * * * * *");
    connect(cron, SIGNAL(activated()),   this, SLOT(onActivation()));
    connect(cron, SIGNAL(deactivated()), this, SLOT(onDeactivation()));
}

void scheduler::onActivation()
{
    log.write("CRON", "ACTIVATED", 0);
}

void scheduler::onDeactivation()
{
    log.write("CRON", "DEACTIVATED", 0);
}
