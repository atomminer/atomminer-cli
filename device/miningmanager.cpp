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

#include "g.h"
#include "stratum/stratum.h"
#include "stratum/stratumwork.h"
#include "utils/log.h"

#include "miningmanager.h"
#include "miner.h"

MininngManager *gpMiningManager = nullptr;

void miningStart()
{
     // the Qt way
    if(gpMiningManager)
        return;

    if(!gpMiningManager)
    {
        QThread *thread = new QThread;
        gpMiningManager = new MininngManager();

        gpMiningManager->moveToThread(thread);
        QObject::connect(thread, SIGNAL(started()), gpMiningManager, SLOT(run()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }
}

MininngManager::MininngManager() : QObject(nullptr)
{
}

void MininngManager::deviceConnected(libusb_device* dev)
{
    log("Device connected");
    QMutexLocker ml(&_lock);

    auto miner = Miner::fromDevice(G::DevMgr.get(dev));
    _miners[dev] = miner;

    connect(miner, SIGNAL(nonceFound(StratumWork *)), this, SLOT(foundNonce(StratumWork *)));
}
void MininngManager::deviceDisconnected(libusb_device* dev)
{
    log("Device disconnected");
    QMutexLocker ml(&_lock);
    auto it = _miners.find(dev);
    if(it != _miners.end())
    {
        delete _miners[dev];
        _miners.erase(it);
    }
}

void MininngManager::foundNonce(StratumWork *w)
{
    // TODO: we should have a stratum manager here and submit it to the right stratum
    //loge("MininngManager::foundNonce");
    stratum()->submit(w);
}

void MininngManager::newJobFromStratum(QString algo)
{
    //logd("MininngManager::newJobFromStratum");
    QMutexLocker ml(&_lock);
    _restart[algo] = true;
}

void MininngManager::run()
{
    connect(G::HotPlugHandler, SIGNAL(deviceAdd(libusb_device*)), this, SLOT(deviceConnected(libusb_device*)), Qt::DirectConnection);
    connect(G::HotPlugHandler, SIGNAL(deviceGone(libusb_device*)), this, SLOT(deviceDisconnected(libusb_device*)), Qt::DirectConnection);

    connect(stratum(), SIGNAL(newJobReceived(QString)), this, SLOT(newJobFromStratum(QString)), Qt::DirectConnection);

    int nSleepMult = 1;

    while (!G::isShuttingDown)
    {
        QThread::msleep(nSleepMult*100);
        nSleepMult = 1;
        QMutexLocker ml(&_lock);

        for (auto it : _restart.toStdMap())
        {
            if(it.second)
            {
                for (const auto& dev : _miners)
                    if(dev->algo() == it.first)
                        dev->sendWork(stratum()->getWork(dev->algo()));

                _restart[it.first] = false;
            }
        }
        for (const auto& dev : _miners)
        {
            dev->update();
            // take care of idle devices
            if(dev->online() && !dev->hashing() && stratum()->hasWork(dev->algo()))
            {
                dev->sendWork(stratum()->getWork(dev->algo()));
                nSleepMult = 3;
            }
        }


    }

    this->deleteLater();
}
