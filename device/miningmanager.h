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

#ifndef MINER_MANAGER_H
#define MINER_MANAGER_H

#include <QMap>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <libusb.h>

class  Miner;
struct StratumWork;

class MininngManager : public QObject
{
    Q_OBJECT
public:
    MininngManager();

protected:
    QMap<libusb_device*, Miner*> _miners;
    QMap<QString, bool> _restart;

    QMutex  _lock;

private slots:
    void deviceConnected(libusb_device*);
    void deviceDisconnected(libusb_device*);
    void foundNonce(StratumWork *w);

public slots:
    void newJobFromStratum(QString algo);

    void run();
};

void miningStart();

#endif // MINER_H
