#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <algorithm>
#include <QCryptographicHash>

#include "stratumwork.h"

#include "utils/fmt/format.h"
#include "utils/log.h"
#include "utils/u.h"

StratumJob::StratumJob()
{
    _coinbase = nullptr;
    memset(_xnonce2, 0, NONCE2_MAX_SIZE);
    _xnonce2[0] = 1;
}

StratumJob::~StratumJob()
{
    _merkle.clear();
    if(_coinbase)
        delete _coinbase;
}

QByteArray StratumJob::reverse(QByteArray arr)
{
    QByteArray reverse;
    for(int i = arr.size() - 1 ; i >= 0 ; i--)
        reverse += arr.at(i);
    return reverse;
}

StratumWork* StratumJob::getNewWork()
{
    StratumWork *w = new StratumWork();

    w->algo = _algo;
    w->jobID = _jobID;
    w->ntime = _ntime;

//    _xnonce2Size = 4; // parsed from subscribe
//    _xnonceSize = 4; // parsed from subscribe
    uint8_t strnonce2[NONCE2_MAX_SIZE];
    for(int i = 0 ; i < _xnonce2Size ; i++)
        strnonce2[i] = _xnonce2[_xnonce2Size - 1 - i];

    w->xnonce2 = QByteArray::fromRawData((const char*)strnonce2, _xnonce2Size).toHex();

    QByteArray coinbase;
    coinbase = QByteArray::fromHex(_coinb1.toLatin1());
    coinbase += QByteArray::fromHex(_xnonce.toLatin1());
    coinbase += QByteArray::fromHex(w->xnonce2.toLatin1());
    coinbase += QByteArray::fromHex(_coinb2.toLatin1());

    int t = _coinb1.length() / 2 + _xnonceSize + _xnonce2Size + _coinb2.length() / 2;
    if(coinbase.size() != t)
    {
        loge("Something is wrong");
        delete w;
        return nullptr;
    }

    //logd(fmt::format("{} -- Generating new work for job {}", _algo.toStdString(), _jobID.toStdString()));

    QByteArray merkle_root;
    if(_algo == "keccak") //groest, blakecoin
    {
        merkle_root = QCryptographicHash::hash(coinbase, QCryptographicHash::Sha256);
    }
    else // the rest. sha256d, keccakc
    {
        merkle_root = QCryptographicHash::hash(coinbase, QCryptographicHash::Sha256);
        merkle_root = QCryptographicHash::hash(merkle_root, QCryptographicHash::Sha256);
    }

    for(int i = 0 ; i < (int)_merkle.size() ; i ++)
    { // verify
        merkle_root += QByteArray::fromHex(_merkle[i].toLatin1());
        merkle_root = QCryptographicHash::hash(merkle_root, QCryptographicHash::Sha256);
        merkle_root = QCryptographicHash::hash(merkle_root, QCryptographicHash::Sha256);
    }

    for(int i = 0 ; i < merkle_root.size() ; i += 4)
        U::bswap32(merkle_root.data()+i);

    w->header.clear();
    w->header.fill(0, 4); // TODO: fill with auth reply from server
    w->header += QByteArray::fromHex(_version.toLatin1());
    w->header += QByteArray::fromHex(_prevhash.toLatin1());
    w->header += merkle_root;
    w->header += QByteArray::fromHex(_ntime.toLatin1());
    w->header += QByteArray::fromHex(_nbits.toLatin1());
    w->header += QByteArray(4, 0); // nonce

    double diff = _diff;
    if(_algo == "keccak")
        diff /= 128.0;
    else if(_algo == "groestl")
        diff /= 256.0;

    int k;
    uint64_t m;

    for (k = 6; k > 0 && diff > 1.0; k--)
        diff /= 4294967296.0;
    m = (uint64_t)(4294901760.0 / diff);
    if (m == 0 && k == 6)
        w->target.fill(0xff, 32);
    else {
        w->target.fill(0, 32);
        ((uint32_t*)w->target.data())[k] = (uint32_t)m;
        ((uint32_t*)w->target.data())[k + 1] = (uint32_t)(m >> 32);
    }

    w->target = reverse(w->target);
    for(int i = 0 ; i < w->target.size() ; i += 4)
        U::bswap32(w->target.data()+i);

    //logd(w->header.toHex().toStdString());
    //logd(w->target.toHex().toStdString());

    // increase nonce2
    int i = 0;
    while(_xnonce2[i] == 0xff && i < _xnonce2Size)
    {
        _xnonce2[i] = 0;
        i++;
    }
    _xnonce2[i] ++;

     return w;
}
