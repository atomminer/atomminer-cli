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

#ifndef __INTEGRITY_H__
#define __INTEGRITY_H__
#include <curl/curl.h>
#include <QMap>
#include <QString>

class Firmware
{
public:
    Firmware();

    void checkLocalFiles();
    bool checkUpdates(bool recheck = false);

    void clearAllFirmware();

    QString getFirmwareForAlgo(QString algo = "");

private:
    QString _getHash(QString file);
    QString _getHashString(QString s);
    void _log(QString s);

    CURL* _initCURL(QString host);

    QString _getUpdateJson(QString which);
    bool _download(QString what, void *helper);

private:
    bool    _printToConsole;
    QMap<QString, QString> _fwMap;
    QMap<QString, QString> _othersMap;

    bool _bUpToDate;
    bool _bLocalFilesChecked;
};

#endif
