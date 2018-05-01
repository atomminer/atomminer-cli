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

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "stratum/stratum.h"
#include "api/api.h"
#include "settings/settings.h"
#include <boost/filesystem.hpp>
#include <QCoreApplication>
#include <QtCore>
#include <QDir>

#include "g.h"

static void signal_handler(int sig)
{
    switch (sig) {
    case SIGHUP:
        printf("SIGHUP received.\n");
        break;
    case SIGINT:
        printf("SIGINT received, exiting\n");
        G::isShuttingDown = true;
        break;
    case SIGTERM:
        printf("SIGTERM received, exiting\n");
        G::isShuttingDown = true;
        break;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (curl_global_init(CURL_GLOBAL_ALL))
        exit(1);

    // catch Ctrl+C and TERM
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::string sPath = "";
    try
    {
        char buf[2048] = {0};
        readlink("/proc/self/exe", buf, 2048);

        boost::filesystem::path path = buf;
        path.remove_filename() /= "atomminer.conf";
        sPath = path.string();
    }
    catch(void*)
    {
        sPath = "";
    }

    if(!conf()->load(sPath))
        conf()->load("");


    QString home = QDir::homePath() + "/.atomminer";

    QDir fwPath(home + "/firmware");
    if(!fwPath.exists())
        fwPath.mkpath(".");

    // TODO: check firmware integrity
    // use QCryptographicHash::hash(fileData, QCryptographicHash::Sha);
    // TODO: check for updated firmware
    // TODO: load preferred device settings


    if(conf()->api())
        api()->start();

    G::HotPlugHandler.start();

    stratum()->connect(conf()->pool(), "mRiZfb1CCEpEsudF9BgUFxWVVtvqMTy4tA", conf()->password());

    int nExit = a.exec();

    G::isShuttingDown = true;
    stratum()->disconnect();
    API::destroy();

    return nExit;
}
