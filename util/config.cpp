#include "config.h"

config::config()
{
}

QStringList config::read(QString obj)
{
    QStringList result;
    if (loadConfig(config::Json)) {
        JsonDoc = QJsonDocument::fromJson(data_json);
        QJsonObject object = JsonDoc.object();
        QJsonValue value = object.value(obj);
        if (obj == "REDIS") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(v.toObject().value("IP").toString());
                    result.append(QString::number(v.toObject().value("PORT").toInt()));
                }
            } else {
                result.append(value.toObject().value("IP").toString());
                result.append(QString::number(value.toObject().value("PORT").toInt()));
            }
        }
        else if (obj == "CONFIG") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(QString::number(v.toObject().value("INTERVAL").toInt()));
                    result.append(QString::number(v.toObject().value("WEBSOCKET_PORT").toInt()));
                    result.append(v.toObject().value("ROOT_TALISA_ASSETS").toString());
                    result.append(v.toObject().value("ROOT_TALISA_API").toString());
                    result.append(QString::number(v.toObject().value("DEBUG_DATA").toBool()));
                    result.append(QString::number(v.toObject().value("DEBUG_DATABASE").toBool()));
                }
            } else {
                result.append(QString::number(value.toObject().value("INTERVAL").toInt()));
                result.append(QString::number(value.toObject().value("WEBSOCKET_PORT").toInt()));
                result.append(value.toObject().value("ROOT_TALISA_ASSETS").toString());
                result.append(value.toObject().value("ROOT_TALISA_API").toString());
                result.append(QString::number(value.toObject().value("DEBUG_DATA").toInt()));
                result.append(QString::number(value.toObject().value("DEBUG_DATABASE").toInt()));
            }
        }
        else if (obj == "EMAIL") {
            if (object.value(obj).isArray()) {
                QJsonArray array = value.toArray();
                foreach (const QJsonValue & v, array) {
                    result.append(v.toObject().value("SENDER").toString());
                    result.append(v.toObject().value("PASSWORD").toString());
                    result.append(v.toObject().value("SERVER").toString());
                    result.append(QString::number(v.toObject().value("PORT").toInt()));
                    result.append(v.toObject().value("RECIPIENT").toString());
                }
            } else {
                result.append(value.toObject().value("SENDER").toString());
                result.append(value.toObject().value("PASSWORD").toString());
                result.append(value.toObject().value("SERVER").toString());
                result.append(QString::number(value.toObject().value("PORT").toInt()));
                result.append(value.toObject().value("RECIPIENT").toString());
            }
        }
        else if (obj == "RPT_GEN") {
          result.append(object.value(obj).toString());
        }
    }
    return result;
}

void config::write(QJsonObject &json) const //Default
{
    QJsonArray redisArray;
    QJsonObject redisObject;
    redisObject["IP"] = QString("127.0.0.1");
    redisObject["PORT"] = 6379;
    redisArray.append(redisObject);
    json["REDIS"] = redisArray;

    QJsonArray configArray;
    QJsonObject configObject;
    configObject["INTERVAL"] = 1000;    //milis
    configObject["WEBSOCKET_PORT"] = 2345;
    configObject["DEBUG_DATA"] = false;
    configObject["DEBUG_DATABASE"] = false;
    configObject["ROOT_TALISA_ASSETS"] = "/var/www/talisa/assets";
    configObject["ROOT_TALISA_API"] = "http://localhost:1965/";
    configArray.append(configObject);
    json["CONFIG"] = configArray;

    QJsonArray emailArray;
    QJsonObject emailObject;
    emailObject["SENDER"] = "sender@gmail.com";
    emailObject["PASSWORD"] = "sender-password-email";
    emailObject["SERVER"] = "smtp.gmail.com";
    emailObject["PORT"] = 465;
    emailObject["RECIPIENT"] = "dendy@daunbiru.com";
    emailArray.append(emailObject);
    json["EMAIL"] = emailArray;

    json["RPT_GEN"] = QString("path/to/report/generator/Report");
}

bool config::loadConfig(config::SaveFormat saveFormat)
{
    QString path;
    if (saveFormat == Json) {
        path = PATH_CONFIGURATION_JSON;
    } else {
        path = PATH_CONFIGURATION_BIN;
    }
    QFile loadFile(path);
    if (!loadFile.exists()) {
        QDir dir;
        dir.mkpath(PATH_MONITA);
    }
    if (loadFile.open(QIODevice::ReadWrite)) {
        QByteArray readFile = loadFile.readAll();
        if (readFile.isEmpty()) {
            QJsonObject configObject;
            write(configObject);
            QJsonDocument saveDoc(configObject);
            loadFile.write(saveFormat == Json
                ? saveDoc.toJson()
                : saveDoc.toBinaryData());
        }
        data_json = readFile;
        return true;
    } else {
        return false;
    }

//    read(loadDoc.object());
}
//! [3]

//! [4]
bool config::saveConfig(config::SaveFormat saveFormat) const
{
    QString path;
    if (saveFormat == Json) {
        path = PATH_CONFIGURATION_JSON;
    } else {
        path = PATH_CONFIGURATION_BIN;
    }
    QFile saveFile(path);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Couldn't save config ..";
        return false;
    }

    QJsonObject configObject;
    write(configObject);
    QJsonDocument saveDoc(configObject);
    saveFile.write(saveFormat == Json
        ? saveDoc.toJson()
        : saveDoc.toBinaryData());

    return true;
}
