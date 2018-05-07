/*
 * AtomMiner CLI miner
 * Copyright AtomMiner, 2018,
 * All Rights Reserved
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF ATOMMINER
 *
 *      Author: AtomMiner - null3128
 */

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <QDir>
#include "utils/log.h"
#include "utils/fmt/format.h"
#include "conf.h"

Conf *Conf::_instance = nullptr;
static QString default_settings = "{\"pool\":\"http://keccak.pool.atomminer.com:5133\",\"user\":\"\",\"password\":\"\","
                                      "\"api\":true,\"api_port\":9123,\"api_ssl\":true,\"api_cert\":\":/api_server.pem\",\"api_cert_key\":\"\",\"api_login\":true,\"api_user\":\"api\",\"api_password\":\"pass\","
                                      "\"proxy\":false,\"proxy_url\":\"\",\"proxy_port\":8080}";

Conf *conf()
{
    return Conf::inst();
}

Conf *Conf::inst()
{
    if(!Conf::_instance)
        Conf::_instance = new Conf();

    return _instance;
}

Conf::Conf()
{
    ;
}

bool Conf::load(QString path)
{
    if(!path.isEmpty() && !QFile::exists(path))
    {
        logw(fmt::format("Error loading config: {}", path.toStdString()));
        return false;
    }

    if(path.isEmpty())
        _config = QJsonDocument::fromJson(default_settings.toUtf8()).object();
    else
    {
        QFile file(path);

        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray fileData = file.readAll();
            _config = QJsonDocument::fromJson(fileData).object();
        }
        else
            _config = QJsonDocument::fromJson("").object();
    }

    if(_config.isEmpty())
    {
        logw(fmt::format("Error loading config: {}", path.toStdString()));
        return false;
    }

    return true;
}

QString Conf::stringValue(QString name, QString def)
{
    if(!_config[name].isString())
        return def;
    return _config[name].toString();
}

bool Conf::boolValue(QString name, bool def)
{
    if(!_config[name].isBool())
        return def;
    return _config[name].toBool();
}

int Conf::intValue(QString name, int def)
{
    if(!_config[name].isDouble())
        return def;
    return (int)_config[name].toDouble();
}

QString Conf::pool()
{
    return stringValue("pool", "http://keccak.pool.atomminer.com:5133");
}

QString Conf::algo()
{
    return stringValue("algo", "");
}

QString Conf::user()
{
    return stringValue("user", "");
}

QString Conf::password()
{
    return stringValue("password", "");
}

bool Conf::api()
{
    return boolValue("api", true);
}

int  Conf::apiPort()
{
    return intValue("api_port", 9123);
}

bool Conf::apiSSL()
{
    return boolValue("api_ssl", false);
}

bool Conf::apiLogin()
{
    return boolValue("api_login", true);
}
QString Conf::apiCert()
{
    return stringValue("api_cert", ":/api_server.pem");
}

QString Conf::apiCertKeyPass()
{
    return stringValue("api_key_pass", "");
}

QString Conf::apiUser()
{
    return stringValue("api_user", "api");
}

QString Conf::apiPassword()
{
    return stringValue("api_password", "pass");
}

bool Conf::proxy()
{
    return boolValue("proxy", false);
}
QString Conf::proxyUrl()
{
    return stringValue("proxy_url", "");
}

int Conf::proxyPort()
{
    return intValue("proxy_port", 0);
}
