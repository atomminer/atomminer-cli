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

#ifndef __MINER_H__
#define __MINER_H__

#include <QMutex>
#include <QObject>
#include <QString>

#include "device.h"

struct StratumWork;

struct MinerStats
{
    QString id;
    QString vidpid;
    QString fw;
    double temp;
    double vcc;
    double vccio;
    bool hashing;
    bool online;

    QString algo;
};

class Miner : public QObject
{
    Q_OBJECT
private:
    explicit Miner(AtomMinerDevice *dev);

public:
    ~Miner();
    static Miner* fromDevice(AtomMinerDevice *dev);

    void getStats(MinerStats *stats);

    QString status();
    QString algo();
    bool canUse();

    bool online();
    bool hashing();

    void update();

    void sendWork(StratumWork *data);

private:
    bool program(QString algo);

private:
    QMutex _mutex;
    AtomMinerDevice *_dev;
    QString _vidpid;
    QString _status;
    QString _id;
    QString _firmware;
    QString _algo;
    double _temp;
    double _vcc;
    double _vccio;

    StratumWork *_work;

    bool _bOnline;
    bool _bHashing;

    uint32_t _lastNonce;
    uint32_t _prevState;

signals:
    void nonceFound(StratumWork *);
    void restartRequested();
};

#endif // MINER_H
