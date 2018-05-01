/*
 * AtomMiner Stratum Lib
 * Copyright AtomMiner, 2018,
 * All Rights Reserved
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF ATOMMINER
 *
 *      Author: AtomMiner - null3128
 */

#ifndef __ATOM_STRATUM_H__
#define __ATOM_STRATUM_H__
#include "json/jansson.hpp"
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <curl/curl.h>
#include <QObject>

#include "utils/types.h"
#include "stratumjob.h"

class Stratum : QObject
{
    Q_OBJECT
private:
    Stratum();

public:
    //Stratum(std::string url, std::string login, std::string pass);
    ~Stratum();

    static Stratum* inst();

    // disconnects from any active pool if any and connects to new supplied pool
    Stratum* connect(std::string url, std::string login, std::string pass);
    Stratum* disconnect();

    bool isConnected();

    // sets the algo(s) we're willing to mine. empty string for autopool
    void setAlgo(std::string algo);

public:
    // get human readable status
    std::string getStatus();

private:
    void thread();

    void _disconnect(std::string reason = "");
    bool _connect();

    bool _socket_full(int timeout);
    bool _send(std::string s);
    std::string _recv();

    bool _subscribe();
    bool _authorize();
    bool _authorize_atom();

    void _process_command(std::string s);

private:
    static Stratum *_instance;
    std::thread *_pThread;

    std::mutex _mutex;

    aclock_t _keepAliveTimer;

    CURL *_curl;
    curl_socket_t _sock;
    char _curlError[256];
    std::string _buffer;

    int _reconnect_wait_time;

    std::string _csrf;

    std::string _session;
    double _diff;
    std::string _extraNonce;
    uint32_t _extraNonceSize;

    bool _bShutdown;        // generally set on app shutdown
    bool _bShutdownDone;

    bool _bDisconnect;      // signal to disconnect from pool
    bool _bConnect;         // signal to connect to pool

    bool _bConnected;

    std::string _status;
    std::string _url;
    std::string _login;
    std::string _pass;

    std::string _algo;

    // stratum jobs
    // todo: delete jobs on pool disconnect
    std::map<std::string, StratumJob*> _jobs;


signals:
    void connected();
    void disconnected(std::string reason);
    void newJobReceived(std::string algo);
};

Stratum* stratum();

#endif
