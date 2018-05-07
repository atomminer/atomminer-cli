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

#include <QFile>
#include <QObject>

#include "g.h"
#include "stratum/stratum.h"
#include "utils/fmt/format.h"
#include "utils/gz.h"
#include "utils/log.h"
#include "utils/u.h"

#include "miner.h"

#define MinerLock  QMutexLocker lock(&_mutex);

Miner::Miner(AtomMinerDevice *dev) : QObject(nullptr)
{
    _dev = dev;
    _bOnline = false;
    _bHashing = false;
    _work = nullptr;
    _prevState = 0;

    _id = "";
    _status = "Unknown";
    _firmware = "";
    _algo = "";

    if(_dev)
        _dev->open();

    _vidpid = QString::fromStdString(fmt::format("{}:{}", _dev->VID, _dev->PID));

    // here's the question...
    // what to do with new device ???
    // option: program blank
    // option: program global algo
    // option: read global device settigns file and go from there?

    // temporary force program to whatever stratum is configured for
    uint32_t buf[32];
    int r = _dev->readCmd(0xb2, 0x06, 0x0, 0, (uint8_t*)buf, 128);
    bool bPorgram = r < 0 || r != 128;
    if(!bPorgram)
        bPorgram = ((buf[3] >> 24) & 0xff) != 1; //keccak

    if(bPorgram)
        program(stratum()->algo());
    else
    {
        uint8_t algo = (uint8_t)((buf[3] >> 24) & 0xff);
        if(algo == 1)
            _algo = "keccak";

        log("YAY! Device is already configured with keccak!");

        // nah...need to test if this is needed
//        QByteArray ba;
//        ba.fill(0, 132);
//        _dev->sendCmd(0x77, 0xf2, 0, 0, ba.data(), ba.size());
    }

    update();
}

Miner::~Miner()
{
    if(_work)
        delete _work;
}

Miner* Miner::fromDevice(AtomMinerDevice *dev)
{
    return new Miner(dev);
}

void Miner::getStats(MinerStats *stats)
{
    if(!stats)
        return;

    MinerLock;
    stats->online = _bOnline;
    stats->hashing = _bHashing;
    stats->algo = _algo;

    stats->id = _id;
    stats->vidpid = _vidpid;
    stats->fw = _firmware;
    stats->temp = _temp;
    stats->vcc = _vcc;
    stats->vccio = _vccio;
}

bool Miner::program(QString algo)
{
    uint8_t io[64];

    if(!_dev)
    {
        _algo = "";
        return false;
    }

    QString fwImage = G::fw.getFirmwareForAlgo(algo);
    QFile file(U::firmware() + "/" + fwImage);

    if(!file.open(QIODevice::ReadOnly))
    {
        loge(fmt::format("Can't read firmware image for {}", algo.toStdString()));
        _algo = "";
        return false;
    }

    QByteArray out;
    auto indata = file.readAll();
    file.close();

    if(!gz::get(indata, out))
    {
        loge(fmt::format("Can't read firmware image for {}", algo.toStdString()));
        _algo = "";
        return false;
    }

    QTime timer;
    timer.start();

    _dev->sendCmd(0x77, 0x14, 0, 0);
    QThread::msleep(10);
    _dev->sendCmd(0x77, 0x16, 0, 0);
    QThread::msleep(5);

    uint32_t idx = 0;
    while(idx < out.size())
    {
        int size = (idx + 32768) < out.size() ? 32768 : out.size() - idx;
        int transferred = _dev->sendData(0x01, (uint8_t*)(out.data() + idx), size);
        idx += transferred;
        logprogress(fmt::format("prg {}", idx));
    }
    out.clear();

    QThread::msleep(10);
    _dev->getIO(io);
    int elapsed = timer.elapsed();

    // TODO:
    // ok, waiting for low level IO is technically incorrect. IO cmd is marked to be removed from release
    // should use getmode cmd
    if(!io[50])
    {
        loge("Failed to program device");
        _algo = "";
        _bOnline = false;
        return false;
    }


    log(QString("Device programmed to %1 in %2ms").arg(algo).arg(elapsed).toStdString());
    _algo = algo;

    return true;
}

QString Miner::status()
{
    MinerLock;
    return _status;
}

QString Miner::algo()
{
    MinerLock;
    return _algo;
}

bool Miner::canUse()
{
    MinerLock;
    if(!_dev || !_bOnline)
        return false;

    return true;
}

bool Miner::online()
{
    MinerLock;
    return _bOnline;
}

bool Miner::hashing()
{
    MinerLock;
    return _bHashing;
}

void Miner::update()
{
    MinerLock;
    if(!_dev)
    {
        _bOnline = false;
        _bHashing = false;
        _status = "Offline";
        return;
    }
    if(!_dev->getLL())
    {
        _bOnline = false;
        _bHashing = false;
        _status = "Offline";
        return;
    }
    if(!_dev->isValid() || !_dev->isConnected())
    {
        _bOnline = false;
        _bHashing = false;
        _status = "Offline";
        return;
    }

    if(!G::DevMgr.has(_dev))
    {
        _bOnline = false;
        _bHashing = false;
        _status = "Offline";
        return;
    }

    if(_firmware.isEmpty())
    {
        _firmware = QString::fromStdString(_dev->firmwareInfo());
        if(_firmware.isEmpty())
        { // should not even be here...ever....something is terribly wrong with this miner. Disconnect
            _dev->close();
            loge(fmt::format("{:X}:{:X} FATAL: device is not configured. Power it off ASAP.", _dev->VID, _dev->PID));
            _bOnline = false;
            _bHashing = false;
            _status = "Offline";
            return;
        }
    }

    // damn stupid debug line. makes controller happy for now.
    // TODO: remove 86 from firmware completely
    uint8_t trash[1024];
    _dev->readData(0x86, trash, 1024);

    _bOnline = true;
    uint32_t buf[32];
    int r = _dev->readCmd(0xb2, 0x06, 0x0, 0, (uint8_t*)buf, 128);
    if(r <= 0 || r != 128)
    { // this thing is misbehaving. disconnect
        _dev->close();
        loge(fmt::format("{:X}:{:X} FATAL: device is not configured. Power it off ASAP.", _dev->VID, _dev->PID));
        _bOnline = false;
        _bHashing = false;
        _status = "Offline";
        return;
    }

    if(0x41746f6d != buf[0] || 0x6d696e65 != buf[1] || buf[31] != 0xffffffff)
    {
        // TODO: add reprogram attempts
        loge("Really want to reprogram device, but won't for now");
        //program(stratum()->algo());
        return;
    }

    _firmware.sprintf("%04x", buf[4]);
    _id.sprintf("%04x%04x", buf[6], buf[7]);
    _temp = (double)(0xffff & (buf[0xa])) / 1115.4168242;
    _vcc = 4.57763671875e-05 * (0xffff & (buf[0xb]));
    _vccio = 4.57763671875e-05 * (0xffff & (buf[0xc]));

    bool wasHashing = (uint8_t)(0xff & (_prevState >> 28));
    _bHashing = (uint8_t)(0xff & (buf[0xd] >> 28));
    uint16_t cplStatus = (uint16_t)(0xffff & ((buf[0xd] >> 12)));

    logprogress(fmt::format("{} - {:.2f}C {:.2f}V {:.2f}V  h:{} {:04x} {:04x}", _id.toStdString(), _temp, _vcc, _vccio, _bHashing, buf[0xd], buf[0xe]));

    if(!_bHashing && wasHashing)
    {
//        if(_lastNonce != buf[0xe])
//        {
//            _lastNonce != buf[0xe];
            if(cplStatus == 0x2121)
            {
                logd(fmt::format("{} found nonce: {:04x}", _id.toStdString(), buf[0xe]));
                _work->nonce = QByteArray::fromRawData((const char*)&buf[0xe], 4).toHex();
                emit nonceFound(_work);
            }
            else
                logd(fmt::format("{} restarting", _id.toStdString()));
    }

    if(_bHashing)
        _status = "Working";
    else
        _status = "Idle";

    _prevState = buf[0xd];
}

void Miner::sendWork(StratumWork *w)
{
    if(!online() || !canUse() || !w)
        return;

    MinerLock;

    if(_work)
        delete _work;
    _work = w;

    QByteArray data;
    data = w->header;
    data += w->target;
    while(data.size() < 128)
        data += (char)0;
    logd(fmt::format("[{}] > {}", _id.toStdString(), data.toHex().toStdString()));
    _dev->sendCmd(0x77, 0xf2, 0, 0, data.data(), data.size());
}
