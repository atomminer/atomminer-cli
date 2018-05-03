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
#include <QCoreApplication>
#include <QtCore>
#include <QDir>

#include "stratum/stratum.h"
#include "api/api.h"
#include "settings/conf.h"
#include "firmware/firmware.h"
#include "utils/fmt/format.h"

#include "g.h"
#include "build.h"

QCoreApplication *pApp = nullptr;

static void signal_handler(int sig)
{
    switch (sig) {
    case SIGHUP:
        printf("SIGHUP received.\n");
        break;
    case SIGINT:
        printf("SIGINT received, exiting\n");
        G::isShuttingDown = true;
        if(pApp)
            pApp->exit(0);
        else
            exit(0);
        break;
    case SIGTERM:
        printf("SIGTERM received, exiting\n");
        G::isShuttingDown = true;
        if(pApp)
            pApp->exit(0);
        else
            exit(0);
        break;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    pApp = &a;

    printf("AtomMiner CLI Miner " PACKAGE_VERSION " \n");
    printf("-- built on %s with %s\n",  BUILD_DATE, COMPILER);

    if (curl_global_init(CURL_GLOBAL_ALL))
    {
        loge("FATAL: Failed to init CURL. Exiting...");
        exit(1);
    }

    // catch Ctrl+C and TERM
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    QString sConf = QCoreApplication::applicationDirPath() + "/atomminer.conf";
    if(argc > 1)
        sConf = argv[1];

    if(!conf()->load(sConf))
        conf()->load("");

    QString home = QDir::homePath() + "/.atomminer";

    QDir fwPath(home + "/firmware");
    if(!fwPath.exists())
        fwPath.mkpath(".");

    Firmware intCheck;
    if(!intCheck.checkUpdates())
    {
        loge("Failed to check for updates. Please check your internet connection and try again.");
        logw("It is not recommended to use miners without latest firmware images");

        int nCnt = 15;
        while (nCnt > 0) {
            logw(fmt::format("Continue at your own risk in {}", nCnt--));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        loge("You've been Warned!!!");
    }

    // TODO: load preferred device settings

    logprogressK("Starting miner...");
    if(conf()->user().isEmpty())
    {
        loge("No username. Please specify username and password in atomminer.conf file. Exiting...");
        return 1;
    }

    if(conf()->api())
        api()->start();

    G::HotPlugHandler.start();

    // TODO: replace with stratum manager
    stratum()->connect(conf()->pool(), conf()->user(), conf()->password());

    int nExit = a.exec();

    G::isShuttingDown = true;
    stratum()->disconnect();
    API::destroy();

    return nExit;
}
