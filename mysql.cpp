#include "mysql.h"

mysql::mysql()
{

}

QSqlDatabase mysql::connect_db(QString name)
{
    QFile db_path(PATH_DB_CONNECTION);
    if (db_path.exists()) {
        QSettings db_sett(PATH_DB_CONNECTION, QSettings::IniFormat);
        host = db_sett.value("HOST").toString();
        db_name = db_sett.value("DATABASE").toString();
        user_name = db_sett.value("USERNAME").toString();
        password = db_sett.value("PASSWORD").toString();

        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", name);
        db.setHostName(host);
        db.setDatabaseName(db_name);
        db.setUserName(user_name);
        db.setPassword(password);

        return (QSqlDatabase) db;
    } else {
        host = "localhost";
        db_name = "monita_alarm";
        user_name = "root";
        password = "root";

        QSettings db_sett(PATH_DB_CONNECTION, QSettings::IniFormat);
        db_sett.setValue("HOST", host.toUtf8());
        db_sett.setValue("DATABASE", db_name.toUtf8());
        db_sett.setValue("USERNAME", user_name.toUtf8());
        db_sett.setValue("PASSWORD", password.toUtf8());

        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", name);
        db.setHostName(host);
        db.setDatabaseName(db_name);
        db.setUserName(user_name);
        db.setPassword(password);

        return (QSqlDatabase) db;
    }
}

void mysql::close(QSqlDatabase db)
{
    QString connection;
    connection = db.connectionName();
    db.close();
    db = QSqlDatabase();
    db.removeDatabase(connection);
}

void mysql::read_data_alarm(QSqlDatabase db, QString type, int debug)
{
//    QString query;
//    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

//    query = "REPLACE INTO " + tb_name + " ( \
//                alarm_id, \
//                titik_ukur_id, \
//                event_time, \
//                desc \
//            ) VALUES ";
//    query = query + data + ";";
////    log.write(type, query, debug);
//    q.prepare(query);
//    q.exec();
}

void mysql::write_alarm_history(QSqlDatabase db, QString tb_name, QString data, QString type, int debug){
    QString query;
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    query = "REPLACE INTO " + tb_name + " ( \
                alarm_id, \
                titik_ukur_id, \
                event_time, \
                desc \
            ) VALUES ";
    query = query + data + ";";
//    log.write(type, query, debug);
    q.prepare(query);
    q.exec();
}
