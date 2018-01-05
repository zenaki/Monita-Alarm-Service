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

QStringList mysql::read_titik_ukur(QSqlDatabase db, int id_user, QString type, int debug)
{
    QStringList result;
    if (db.isValid()) {
        if (!db.isOpen()) {
            db.open();
        }
        QString query;
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));
        query = "call get_titik_ukur('"+type+"',"+QString::number(id_user)+")";
//        q.prepare(query);
        if (q.exec(query)) {
            while(q.next()) {
                result.append(q.value(0).toString());
                result.append(q.value(1).toString());
            }
        } else {
            log.write("MySQL", "Query fail : " + query, debug);
        }
        db.close();
    }
    return result;
}

QStringList mysql::read_data_alarm(QSqlDatabase db, QString type, int debug)
{
    Q_UNUSED(type);
    QStringList result;
    if (db.isValid()) {
        if (!db.isOpen()) {
            db.open();
        }
        QString query;
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));
        query = "call get_alarm_rules()";
//        q.prepare(query);
        if (q.exec(query)) {
            while(q.next()) {
                result.append(q.value(0).toString());
                result.append(q.value(1).toString());
                result.append(q.value(2).toString());
                result.append(q.value(3).toString());
                result.append(q.value(4).toString());
                result.append(q.value(5).toString());
                result.append(q.value(6).toString());
                result.append(q.value(7).toString());
                result.append(q.value(8).toString());
            }
        } else {
            log.write("MySQL", "Query fail : " + query, debug);
        }
        db.close();
    }
    return result;
}

void mysql::write_alarm_history(QSqlDatabase db, QString data, QString type, int debug)
{
    Q_UNUSED(type);
    if (db.isValid()) {
        if (!db.isOpen()) {
            db.open();
        }

        QString query;
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));

        query = "call set_history_alarm(\'"+data+"\');";

//        log.write(type, query, debug);
//        q.prepare(query);
//        qDebug() << query;
        if (q.exec(query)) {
            log.write("MySQL", "Querry success : " + query, debug);
        } else {
            log.write("MySQL", "Querry fail : " + query, debug);
        }

        db.close();
    }
}

QStringList mysql::read_email(QSqlDatabase db, QString titik_ukur, QString type, int debug)
{
//    Q_UNUSED(type);
    QStringList result;
    if (db.isValid()) {
        if (!db.isOpen()) {
            db.open();
        }
        QString query;
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));
        query = "call get_email('"+titik_ukur+"')";
//        q.prepare(query);
        if (q.exec(query)) {
            while(q.next()) {
                result.append(q.value(0).toString());
                result.append(q.value(1).toString());
            }
        } else {
            log.write(type, "Query fail : " + query, debug);
        }
        db.close();
    }
    return result;
}

QStringList mysql::read_cron(QSqlDatabase db, QString type, int debug)
{
    QStringList result;
    if (db.isValid()) {
        if (!db.isOpen()) {
            db.open();
        }
        QString query;
        QSqlQuery q(QSqlDatabase::database(db.connectionName()));
        query = "call get_cron()";
        if (q.exec(query)) {
            while(q.next()) {
                result.append(q.value(0).toString());
                result.append(q.value(1).toString());
                result.append(q.value(2).toString());
            }
        } else {
            log.write(type, "Query fail : " + query, debug);
        }
        db.close();
    }
    return result;
}
