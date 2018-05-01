#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "stratumjob.h"

#include "utils/fmt/format.h"
#include "utils/log.h"
#include "utils/u.h"

#include "cryptopp/cryptlib.h"
#include "cryptopp/sha.h"

unsigned char *SHA256(const unsigned char *d, size_t n, unsigned char *md);

StratumJob::StratumJob()
{
    _coinbase = nullptr;
}

StratumJob::~StratumJob()
{
    _merkle.clear();
    if(_coinbase)
        delete _coinbase;
}

StrtumWork* StratumJob::getBlockHeader()
{
    StrtumWork *w = new StrtumWork();

    w->_algo = _algo;
    w->_jobID = _jobID;
    w->_ntime = _ntime;

    uint8_t merkle_root[64] = { 0 };

    int coinbase_size = _coinb1.length() / 2 + _xnonceSize + _xnonce2Size + _coinb2.length() / 2;
    if(!_coinbase)
    {
        _coinbase = (uint8_t*)malloc(coinbase_size);
        memset(_coinbase, 0, coinbase_size);
        U::bin(_coinbase, _coinb1);
        U::bin(_coinbase + _coinb1.length() / 2, _xnonce);
        U::bin(_coinbase + _coinb1.length() / 2 + _xnonceSize + _xnonce2Size, _coinb2);
    }

    // increase nonce2
    uint8_t *nonce2 = _coinbase + _coinb1.length() / 2 + _xnonceSize;

    int i = 0;
    while(nonce2[i] == 0xff && i < _xnonce2Size)
    {
        nonce2[i] = 0;
        i++;
    }
    nonce2[i] ++;
    w->_xnonce2 =U::hex(nonce2, _xnonce2Size);

    logd(fmt::format("New work [{}] for job {}", w->_xnonce2, _jobID));

    if(_algo == "keccak") //groest, blakecoin
    {
        CryptoPP::SHA256().CalculateDigest(merkle_root, _coinbase, coinbase_size);
    }
    else
    {
        CryptoPP::SHA256().CalculateDigest(merkle_root, _coinbase, coinbase_size);
        CryptoPP::SHA256().CalculateDigest(merkle_root, merkle_root, CryptoPP::SHA256::DIGESTSIZE);
    }

    for(int i = 0 ; i < (int)_merkle.size() ; i ++)
    {
        U::bin(merkle_root + 32, _merkle[i]);
        CryptoPP::SHA256().CalculateDigest(merkle_root, merkle_root, 64);
        CryptoPP::SHA256().CalculateDigest(merkle_root, merkle_root, CryptoPP::SHA256::DIGESTSIZE);
    }

    memset(w->header, 0, 128);
    w->header[0] = U::le32(_version);
    uint32_t tmp[128];

    U::bin((uint8_t *)&tmp, _prevhash);
    for (i = 0; i < 8; i++)
        w->header[1 + i] = U::le32dec((uint32_t *)tmp + i);
    for (i = 0; i < 8; i++)
        w->header[9 + i] = U::be32dec((uint32_t *) merkle_root + i);

    w->header[17] = U::le32(_ntime);
    w->header[18] = U::le32(_nbits);

    // calculate target
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
        memset(w->target, 0xff, 32);
    else {
        memset(w->target, 0, 32);
        w->target[k] = (uint32_t)m;
        w->target[k + 1] = (uint32_t)(m >> 32);
    }

    logd(fmt::format("new work target: {}", U::hex((uint8_t*)w->target, 32)));

    return w;
}
