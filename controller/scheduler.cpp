#include "scheduler.h"

scheduler::scheduler(QObject *parent) : QObject(parent)
{
    QStringList temp = cfg.read("CONFIG");
    debugData = temp.at(2).toInt();
    debugDatabase = temp.at(3).toInt();

    db = database.connect_db("CRONJOB");
    QStringList result = database.read_cron(db, "CRON", debugDatabase);
    for (int i = 0; i < result.length(); i+=3) {
        
    }
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
//    QCron cron("* * * * * *");

    SigMapCron_Activated = new QSignalMapper(this);
    SigMapCron_Deactivated = new QSignalMapper(this);
//    QCron *cron = new QCron("* 13 * * * *");
    m_cron << new QCron("* 13 * * * *");

    SigMapCron_Activated->setMapping(m_cron.at(m_cron.length()-1), m_cron.length()-1);
    connect(m_cron.at(m_cron.length()-1), SIGNAL(activated()), SigMapCron_Activated, SLOT(map()));
    connect(SigMapCron_Activated,SIGNAL(mapped(int)),this,SLOT(onDeactivation(int)));

    SigMapCron_Deactivated->setMapping(m_cron.at(m_cron.length()-1), m_cron.length()-1);
    connect(m_cron.at(m_cron.length()-1), SIGNAL(deactivated()), SigMapCron_Deactivated, SLOT(map()));
    connect(SigMapCron_Deactivated,SIGNAL(mapped(int)),this,SLOT(onActivation(int)));

//    connect(cron, SIGNAL(activated()),   this, SLOT(onActivation()));
//    connect(cron, SIGNAL(deactivated()), this, SLOT(onDeactivation()));
}

void scheduler::onActivation(int index)
{
    log.write("CRON", "ACTIVATED", debugData);
}

void scheduler::onDeactivation(int index)
{
    log.write("CRON", "DEACTIVATED", debugData);
}

void scheduler::generate_report(int index, QString temp, QString conf, QString name, QString parameter)
{
    QSettings db_sett(PATH_DB_CONNECTION, QSettings::IniFormat);
    QString host = db_sett.value("HOST").toString();
    QString db_name = db_sett.value("DATABASE").toString();
    QString user_name = db_sett.value("USERNAME").toString();
    QString password = db_sett.value("PASSWORD").toString();

    QStringList rpt_gen = cfg.read("RPT_GEN");

    log.write("Process", rpt_gen.at(0) +
              " -tmp " + temp +
              " -cnf " + conf +
              " -f " + name +
              " -host " + host +
              " -db " + db_name +
              " -usr " + user_name +
              " -pwd " + password +
              " -par " + parameter, debugData);
    QProcess proc;
    proc.start("xvfb-run -- /" + rpt_gen.at(0) +
               " -tmp " + temp +
               " -cnf " + conf +
               " -f " + name +
               " -host " + host +
               " -db " + db_name +
               " -usr " + user_name +
               " -pwd " + password +
               " -par " + parameter);
    proc.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
    QString output(proc.readAllStandardOutput());
//    for (int i = 0; i < 1000; i++) {
//        qDebug() << output;
//    }
    output.mid(output.indexOf("{"));
    QJsonObject obj = this->ObjectFromString(output);
    if (obj.value("ERR").isUndefined()) {
//        m_clients.at(index)->sendTextMessage("{\"REPORT\": \""+ obj.value("SUCCESS").toString() +"\"}");
    } else {
//        m_clients.at(index)->sendTextMessage("{\"REPORT\": \""+ obj.value("ERR").toString() +"\"}");
    }
}

QJsonObject scheduler::ObjectFromString(QString in)
{
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

    // check validity of the document
    if (!doc.isNull()) {
        if (doc.isObject()) {
            obj = doc.object();
        }
    }

    return obj;
}
