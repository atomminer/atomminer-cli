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

#ifndef __API_H__
#define __API_H__

#include <mutex>
#include <string>
#include <thread>

class API
{
public:
    API();
    ~API();

    void start();
    void stop();

    static API* inst();
    static void destroy();

private:
    void thread();

private:
    static API *_instance;
    std::thread *_pThread;
    std::mutex _mutex;

    bool _bShutdown;
    bool _bShutdownDone;
};

API *api();

#endif
