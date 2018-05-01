#ifndef STRATUMJOB_H
#define STRATUMJOB_H

#include <mutex>
#include <string>
#include <vector>

#define NONCE2_MAX_SIZE 16

struct StrtumWork
{
    std::string _algo;
    std::string _jobID;
    std::string _ntime;

    uint32_t header[48];
    uint32_t target[8];

    std::string _xnonce2;

//    uint8_t _xnonce2[NONCE2_MAX_SIZE]; // max supported extranonce2
//    int     _xnonce2Size;
};

class StratumJob
{
public:
    StratumJob();
    ~StratumJob();

    // returns new block header for mining every time being called
    StrtumWork* getBlockHeader();

    void construct();

public:
    std::string _algo;
    std::string _jobID;
    std::string _prevhash;
    std::string _coinb1;
    std::string _coinb2;
    std::string _version;
    std::string _nbits;
    std::string _ntime;
    double _diff;
    std::vector<std::string> _merkle;

    std::string _xnonce;
    uint32_t _xnonceSize;

    uint8_t _xnonce2[NONCE2_MAX_SIZE]; // max supported extranonce2
    int     _xnonce2Size;
    uint8_t *_coinbase;
};

#endif // STRATUMJOB_H
