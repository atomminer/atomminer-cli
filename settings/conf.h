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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <QString>

#if QT_VERSION >= 0x050000
#include <QJsonArray>
#include <QJsonDocument>
#else
#include "json/qjsonarray.h"
#include "json/qjsonobject.h"
#include "json/qjsondocument.h"
#endif

class Conf
{
public:
    Conf();

    static Conf *inst();

    bool load(QString path);

    QString pool();
    QString user();
    QString password();

    bool api();
    int  apiPort();
    bool apiSSL();
    bool apiLogin();
    QString apiCert();
    QString apiUser();
    QString apiPassword();

    bool proxy();
    QString proxyUrl();
    int proxyPort();

private:
    QString _stringValue(QString name, QString def);
    bool _boolValue(QString name, bool def);
    int _intValue(QString name, int def);

private:
    static Conf *_instance;
    QJsonObject _config;

};

Conf *conf();

#endif