#ifndef STRATUMTEST_H
#define STRATUMTEST_H

#include "stratum.h"

class StratumTest : public Stratum
{
public:
    StratumTest();

public:
    int sendNewJobCounter;
    int maxNewJobs;

    void setupTestBuffer();

    void _disconnect(std::string reason = "");
    bool _connect();

    bool _socket_full(int timeout);
    bool _send(std::string s);
    std::string _recv();

    bool _subscribe();
    bool _authorize();
    bool _authorize_atom();
};

#endif // STRATUMTEST_H
