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

#include <QDir>
#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QJsonArray>
#include <QJsonDocument>
#else
#include "json/qjsonarray.h"
#include "json/qjsonobject.h"
#include "json/qjsondocument.h"
#endif
#include <QCryptographicHash>
#include <thread>
#include <curl/curl.h>
#include <curl/easy.h>

#include "firmware.h"
#include "settings/conf.h"
#include "utils/fmt/format.h"
#include "utils/log.h"
#include "utils/u.h"

#include "build.h"

#define CURRENT_SUPPORTED_REMOTE_CONF_VERSION  (1)

#define CAN_PRINT_TO_CONSOLE 1

//#define PRINT_FILES

typedef struct {
    FILE *fh;
    uint32_t filesize;
    uint32_t done;
    QString *pMsg;
} __Firmware_download_helper;

static size_t CURL_Firmware_WriteToStringCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((QString*)userp)->append(QByteArray::fromRawData((const char*)contents, size * nmemb));
    return size * nmemb;
}

static size_t CURL_Firmware_downloadCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    __Firmware_download_helper *helper = (__Firmware_download_helper*)userp;

    size_t written = fwrite(contents, size, nmemb, helper->fh);
    helper->done += written;

    if(helper->pMsg)
        logprogress(fmt::format("{} {:.2f}%", qPrintable(*helper->pMsg), 100.0 * (double) helper->done / helper->filesize));

    return written;
}


Firmware::Firmware()
{
#ifdef CAN_PRINT_TO_CONSOLE
    _printToConsole = true;
#else
    _printToConsole = false;
#endif
    _bUpToDate = false;
    _bLocalFilesChecked = false;
}

void Firmware::_log(QString s)
{
    logd(qPrintable(s));
}

QString Firmware::_getHash(QString filename)
{
    QFile file(filename);

    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray fileData = file.readAll();
        QByteArray hashData = QCryptographicHash::hash(fileData, QCryptographicHash::Sha1);
        return QString(hashData.toHex());
    }
    return "";
}

QString Firmware::_getHashString(QString s)
{
    return QString(QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Sha1).toHex());
}

CURL* Firmware::_initCURL(QString host)
{
    CURL* curl = curl_easy_init();

    // WARN: unsecure connection over HTTP will throw 403 or 301 to HTTPS
    curl_easy_setopt(curl, CURLOPT_URL, qPrintable(host));
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION , 1);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    // I don't see why could it take more than 2 redirects...even though 2 is more than enough
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 2L);
    if(conf()->proxy())
    {
        curl_easy_setopt(curl, CURLOPT_PROXY, qPrintable(conf()->proxyUrl()));   // replace with your actual proxy
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, qPrintable(conf()->proxyPort()));
    }

    return curl;
}

QString Firmware::_getUpdateJson(QString which)
{
    if(!which.size())
        return "";

    QString sUpdates = "";

    QString sHost = "https://static.atomminer.com" + which;

    CURL* curl = _initCURL(sHost);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURL_Firmware_WriteToStringCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &sUpdates);

    int rc = curl_easy_perform(curl);
    long httpCode;

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (rc || httpCode >= 400 || sUpdates.length() == 0)
        return "";



    return sUpdates;
}

bool Firmware::_download(QString what, void *h)
{
    if(!what.length() || !h)
        return false;

    __Firmware_download_helper *helper = (__Firmware_download_helper*)h;

    QString sUpdates = "";

    QString sHost = "https://static.atomminer.com" + what;

    CURL* curl = _initCURL(sHost);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURL_Firmware_downloadCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, h);

    int rc = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(rc)
        return false;

    if(helper->done != helper->filesize)
        return false;

    return true;
}

void Firmware::checkLocalFiles()
{
    if(_printToConsole)
        logprogress("Checking system integrity...");
    _fwMap.clear();
    _othersMap.clear();

    QDir directory(U::firmware());
    QStringList files = directory.entryList(QStringList() << "*.*", QDir::Files);
    foreach(QString filename, files)
    {
        if(_printToConsole)
            logprogress(fmt::format("Checking system integrity [{}]", qPrintable(filename)));
        _fwMap[_getHash(U::firmware() + filename)] = filename;
    }

    if(_printToConsole)
        logprogress("Checking system integrity...");

    directory = QDir(U::files());
    files = directory.entryList(QStringList() << "*.*", QDir::Files);
    foreach(QString filename, files)
    {
        if(_printToConsole)
            logprogress(fmt::format("Checking system integrity [{}]", qPrintable(filename)));
        _othersMap[_getHash(U::files() + filename)] = filename;
    }

    if(_printToConsole)
        logprogressK("Checking system integrity...Done");

#ifdef PRINT_FILES
    if(_printToConsole)
    {
        foreach (QString h, _fwMap.keys())
            _log(_fwMap[h] + " " + h);
        foreach (QString h, _othersMap.keys())
            _log(_othersMap[h] + " " + h);
    }
#endif

    _bLocalFilesChecked = true;
}

bool Firmware::checkUpdates(bool recheck)
{
    _bUpToDate = false;

    if(!_bLocalFilesChecked || recheck)
        checkLocalFiles();

    QString sJson = "";
    QString sPath = "/firmware/current.json";
    QJsonDocument j;

    // try a few times...just in case
    int nAttempt = 0;
    while (nAttempt++ < 4)
    {
        if(_printToConsole) // kinda simulating progress output
            logprogress("Checking for updates.");

         sJson = _getUpdateJson(sPath);

        if(_printToConsole)
            logprogress("Checking for updates..");

        if(!sJson.size())
            continue;

        j = QJsonDocument::fromJson(sJson.toUtf8());

        if(j.isNull() || j.isEmpty())
            continue;

        if(!j.object().contains("version"))
            continue;

        if((int)j.object()["version"].toDouble() != CURRENT_SUPPORTED_REMOTE_CONF_VERSION)
        {
            sPath = j.object()["version"].toString();
            continue;
        }

        break;
    }

    if(_printToConsole)
        logprogressK("Checking for updates...Done");

    if(!sJson.size() || j.isNull() || j.isEmpty())
        return false;

    if(!j.object().contains("version"))
        return false;
    if(!j.object().contains("images"))
        return false;
    if(!j.object()["images"].isArray())
        return false;
    if((int)j.object()["version"].toDouble() != CURRENT_SUPPORTED_REMOTE_CONF_VERSION)
        return false;

    QString location = j.object()["files"].toString();
    auto images = j.object()["images"].toArray();
    for(int i = 0 ; i < images.size() ; i++)
    {
        auto img = images[i].toObject();
        if(_fwMap.end() == _fwMap.find(img["tag"].toString()))
        {
            __Firmware_download_helper helper;
            auto file = img["file"].toString();
            QString sFile = U::firmware() + file;

            helper.done = 0;
            helper.filesize = (uint32_t)img["size"].toDouble();
            helper.pMsg = new QString("  Downloading " + file);
            helper.fh = fopen(qPrintable(sFile), "w+");

            bool bResult = _download(location + file, (void*)&helper);

            fclose(helper.fh);

            if(!bResult || img["tag"].toString() != _getHash(sFile))
            {
                logprogressF(qPrintable(*helper.pMsg + " - Failed"));
                logw(fmt::format("Failed to download {}", qPrintable(file)));
                QFile::remove(sFile);
                delete helper.pMsg;
                continue;
            }

            logprogressK(qPrintable(*helper.pMsg + " - OK"));
            delete helper.pMsg;
        }
    }

    _bUpToDate = true;
    return true;
}
