#include "scheduler.h"

scheduler::scheduler(QObject *parent, notification *n) : QObject(parent)
{
//    manager = new QNetworkAccessManager(this);
//    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply *)));

    QStringList temp = cfg.read("CONFIG");
    root_talisa_api = temp.at(3);
    debugData = temp.at(4).toInt();
    debugDatabase = temp.at(5).toInt();

    temp = cfg.read("EMAIL");
    email_sender = temp.at(0);
    email_password = temp.at(1);
    email_server = temp.at(2);
    email_port = temp.at(3).toInt();
    email_recipient = temp.at(4);

    notf = n;

    db = database.connect_db("CRONJOB");
    QStringList result = database.read_cron(db, "CRON", debugDatabase);
    for (int i = 0; i < result.length(); i+=4) {
        if (result.at(i+1).count(QRegExp("^\\s*($|#|\\w+\\s*=|(\\?|\\*|(?:\\*|[0-5]?\\d)(?:(?:-|\\/|\\,)(?:[0-5]?\\d))?(?:,(?:[0-5]?\\d)(?:(?:-|\\/|\\,)(?:[0-5]?\\d))?)*)\\s+(\\?|\\*|(?:[0-5]?\\d)(?:(?:-|\\/|\\,)(?:[0-5]?\\d))?(?:,(?:[0-5]?\\d)(?:(?:-|\\/|\\,)(?:[0-5]?\\d))?)*)\\s+(\\?|\\*|(?:[01]?\\d|2[0-3])(?:(?:-|\\/|\\,)(?:[01]?\\d|2[0-3]))?(?:,(?:[01]?\\d|2[0-3])(?:(?:-|\\/|\\,)(?:[01]?\\d|2[0-3]))?)*)\\s+(\\?|\\*|(?:0?[1-9]|[12]\\d|3[01])(?:(?:-|\\/|\\,)(?:0?[1-9]|[12]\\d|3[01]))?(?:,(?:0?[1-9]|[12]\\d|3[01])(?:(?:-|\\/|\\,)(?:0?[1-9]|[12]\\d|3[01]))?)*)\\s+(\\?|\\*|(?:[1-9]|1[012])(?:(?:-|\\/|\\,)(?:[1-9]|1[012]))?(?:L|W)?(?:,(?:[1-9]|1[012])(?:(?:-|\\/|\\,)(?:[1-9]|1[012]))?(?:L|W)?)*|\\?|\\*|(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)(?:(?:-)(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC))?(?:,(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)(?:(?:-)(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC))?)*))$"))) {
            data_cron dCron;
            dCron.name = result.at(i);
            dCron.time = result.at(i+1);
            dCron.path_visualGroup = result.at(i+2);
            dCron.email = result.at(i+3).split(";");
            dCron.last_execute = QDateTime::currentDateTime();
            m_data_cron << dCron;
        }
    }

//    cron5RegEx = "^\\s*("
//                 "$|#|\\w+\\s*=|(\\?|\\*|(?:[0-5]?\\d)(?:(?:-|\/|\\,)(?:[0-5]?\\d))?(?:,(?:[0-5]?\\d)(?:(?:-|\/|\\,)(?:[0-5]?\\d))?)*)\\s+"
//                 "(\\?|\\*|(?:[0-5]?\\d)(?:(?:-|\/|\\,)(?:[0-5]?\\d))?(?:,(?:[0-5]?\\d)(?:(?:-|\/|\\,)(?:[0-5]?\\d))?)*)\\s+"
//                 "(\\?|\\*|(?:[01]?\\d|2[0-3])(?:(?:-|\/|\\,)(?:[01]?\\d|2[0-3]))?(?:,(?:[01]?\\d|2[0-3])(?:(?:-|\/|\\,)(?:[01]?\\d|2[0-3]))?)*)\\s+"
//                 "(\\?|\\*|(?:0?[1-9]|[12]\\d|3[01])(?:(?:-|\/|\\,)(?:0?[1-9]|[12]\\d|3[01]))?(?:,(?:0?[1-9]|[12]\\d|3[01])(?:(?:-|\/|\\,)(?:0?[1-9]|[12]\\d|3[01]))?)*)\\s+"
//                 "(\\?|\\*|(?:[1-9]|1[012])(?:(?:-|\/|\\,)(?:[1-9]|1[012]))?(?:L|W)?(?:,(?:[1-9]|1[012])(?:(?:-|\/|\\,)(?:[1-9]|1[012]))?(?:L|W)?)*|\\?|\\*|(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)(?:(?:-)(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC))?(?:,(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)(?:(?:-)(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC))?)*)"
//                 ")$";
//    cron6RegEx = "^\\s*("
//                 "$|#|\\w+\\s*=|(\\?|\\*|(?:[0-5]?\\d)(?:(?:-|\/|\\,)(?:[0-5]?\\d))?(?:,(?:[0-5]?\\d)(?:(?:-|\/|\\,)(?:[0-5]?\\d))?)*)\\s+"
//                 "(\\?|\\*|(?:[0-5]?\\d)(?:(?:-|\/|\\,)(?:[0-5]?\\d))?(?:,(?:[0-5]?\\d)(?:(?:-|\/|\\,)(?:[0-5]?\\d))?)*)\\s+"
//                 "(\\?|\\*|(?:[01]?\\d|2[0-3])(?:(?:-|\/|\\,)(?:[01]?\\d|2[0-3]))?(?:,(?:[01]?\\d|2[0-3])(?:(?:-|\/|\\,)(?:[01]?\\d|2[0-3]))?)*)\\s+"
//                 "(\\?|\\*|(?:0?[1-9]|[12]\\d|3[01])(?:(?:-|\/|\\,)(?:0?[1-9]|[12]\\d|3[01]))?(?:,(?:0?[1-9]|[12]\\d|3[01])(?:(?:-|\/|\\,)(?:0?[1-9]|[12]\\d|3[01]))?)*)\\s+"
//                 "(\\?|\\*|(?:[1-9]|1[012])(?:(?:-|\/|\\,)(?:[1-9]|1[012]))?(?:L|W)?(?:,(?:[1-9]|1[012])(?:(?:-|\/|\\,)(?:[1-9]|1[012]))?(?:L|W)?)*|\\?|\\*|(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)(?:(?:-)(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC))?(?:,(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)(?:(?:-)(?:JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC))?)*)\\s+"
//                 "(\\?|\\*|(?:[0-6])(?:(?:-|\/|\\,|#)(?:[0-6]))?(?:L)?(?:,(?:[0-6])(?:(?:-|\/|\\,|#)(?:[0-6]))?(?:L)?)*|\\?|\\*|(?:MON|TUE|WED|THU|FRI|SAT|SUN)(?:(?:-)(?:MON|TUE|WED|THU|FRI|SAT|SUN))?(?:,(?:MON|TUE|WED|THU|FRI|SAT|SUN)(?:(?:-)(?:MON|TUE|WED|THU|FRI|SAT|SUN))?)*)(|\\s)+(\\?|\\*|(?:|\\d{4})(?:(?:-|\/|\\,)(?:|\\d{4}))?(?:,(?:|\\d{4})(?:(?:-|\/|\\,)(?:|\\d{4}))?)*)"
//                 ")$";
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
    for (int i = 0; i < m_data_cron.length(); i++) {
//        QCron *cron = new QCron("0 0 * * 1 *");
        m_cron << new QCron(m_data_cron.at(i).time + " *");

        SigMapCron_Activated->setMapping(m_cron.at(m_cron.length()-1), m_cron.length()-1);
        connect(m_cron.at(m_cron.length()-1), SIGNAL(activated()), SigMapCron_Activated, SLOT(map()));
        connect(SigMapCron_Activated, SIGNAL(mapped(int)), this, SLOT(onActivation(int)));

        SigMapCron_Deactivated->setMapping(m_cron.at(m_cron.length()-1), m_cron.length()-1);
        connect(m_cron.at(m_cron.length()-1), SIGNAL(deactivated()), SigMapCron_Deactivated, SLOT(map()));
        connect(SigMapCron_Deactivated, SIGNAL(mapped(int)), this, SLOT(onDeactivation(int)));

        m_manager_cron << new QNetworkAccessManager(this);
        connect(m_manager_cron.at(m_manager_cron.length()-1), SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply *)));
//        QNetworkReply *rply;
//        m_reply_cron << rply;
//        QNetworkAccessManager *mngr = m_manager_cron.at(m_manager_cron.length()-1);
//        int id = m_manager_cron.length()-1;
//        SigMapManager_Finished->setMapping(m_manager_cron.at(m_manager_cron.length()-1), m_manager_cron.length()-1);
//        connect(m_manager_cron.at(m_manager_cron.length()-1), SIGNAL(finished(QNetworkReply*)), SigMapManager_Finished, SLOT(map()));
//        connect(SigMapManager_Finished, SIGNAL(mapped(int)), this, SLOT(replyFinished(int)));
    }

//        connect(cron, SIGNAL(activated()),   this, SLOT(onActivation()));
//        connect(cron, SIGNAL(deactivated()), this, SLOT(onDeactivation()));

}

void scheduler::onActivation(int index)
{
    if (m_data_cron.at(index).last_execute.toTime_t()+60 <= QDateTime::currentDateTime().toTime_t()) {
        m_data_cron[index].last_execute = QDateTime::currentDateTime();
        log.write("CRON", "ACTIVATED : "+m_data_cron.at(index).name, debugData);
        QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH:mm:ss");
        QNetworkRequest request;
    //    http://119.18.154.235:1965/command/report/pdf/cron?user_id=163&visualMonita=tlnagaharianfix.json&name=ntes
        QString urls = root_talisa_api +
                "command/report/pdf/cron?visualMonita=" +
                m_data_cron.at(index).path_visualGroup +
                "&name="+m_data_cron.at(index).name;
        log.write("CRON", "Report URL : "+urls, debugData);
        QUrl url =  QUrl::fromEncoded(urls.toUtf8().data());

    //    log.write("CRON", "URL Report : " + urls, debugData);
        request.setUrl(url);
    //    QNetworkReply *reply = manager->get(request);
    //    manager->get(request);
    //    m_reply_cron[index] = m_manager_cron.at(index)->get(request);
    //    m_reply_cron.replace(index, m_manager_cron.at(index)->get(request));
        m_manager_cron.at(index)->get(request);
    //    bool ok = false;
    //    while(!reply->isFinished()) {
    //        QByteArray data = reply->readAll();
    //        QJsonObject obj = ObjectFromString(data);
    //        if (!obj.value("path").isUndefined()) {
    //            log.write("CRON", "Report : " + obj.value("path").toString(), debugData);
    //            for (int i = 0; i < m_data_cron.at(index).email.length(); i++) {
    //                notf->sendMail(email_sender,
    //                               email_password,
    //                               email_server,
    //                               email_port,
    //                               m_data_cron.at(index).email.at(i),
    //                               "Monita - Report Scheduler",
    //                               "Report: "+dt,
    //                               QStringList(obj.value("path").toString()));
    //            }
    //            ok = true;
    //        }
    //    }
    //    if (!ok) {
    //        log.write("CRON", "Failed Generate Report! : " + urls, debugData);
    //    }
    }
}

void scheduler::onDeactivation(int index)
{
    Q_UNUSED(index);
//    log.write("CRON", "DEACTIVATED : "+m_data_cron.at(index).name, debugData);
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

void scheduler::replyFinished(QNetworkReply *reply)
//void scheduler::replyFinished(int)
{
    for (int i = 0; i < m_manager_cron.length(); i++) {
        if (reply->manager() == m_manager_cron.at(i)) {
            QByteArray data;
            data = reply->readAll();
            log.write("CRON", data, debugData);
            QJsonObject obj = ObjectFromString(data);
            if (!obj.value("path").isUndefined()) {
                log.write("CRON", "Report : " + obj.value("path").toString(), debugData);
                for (int j = 0; j < m_data_cron.at(i).email.length(); j++) {
                    notf->sendMail(email_sender,
                                   email_password,
                                   email_server,
                                   email_port,
                                   m_data_cron.at(i).email.at(j),
                                   "Monita - Report Scheduler",
                                   "Report: "+QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"),
                                   QStringList(obj.value("path").toString()));
//                                   QStringList("../report-scheduler.pdf"));
                }
            }
        }
    }
}
