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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class Conf
{
public:
    Conf();

    static Conf *inst();

    bool load(QString path);

    QString pool();
    QString algo();
    QString user();
    QString password();

    bool api();
    int  apiPort();
    bool apiSSL();
    bool apiLogin();
    QString apiCert();
    QString apiCertKeyPass();
    QString apiUser();
    QString apiPassword();

    bool proxy();
    QString proxyUrl();
    int proxyPort();

public:
    QString stringValue(QString name, QString def);
    bool boolValue(QString name, bool def);
    int intValue(QString name, int def);

private:
    static Conf *_instance;
    QJsonObject _config;

};

Conf *conf();

#endif
