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

#include "api/api.h"
//#include "settings/settings.h"

API *API::_instance = nullptr;

API *api()
{
    return API::inst();
}

API* API::inst()
{
    if(!API::_instance)
        API::_instance = new API();
    return API::_instance;
}

void API::destroy()
{
    if(API::_instance)
    {
        API::_instance->stop();
        delete API::_instance;
        API::_instance = nullptr;
    }
}

API::API()
{
    _bShutdownDone = true;
    _bShutdown = false;
    _pThread = nullptr;
}

API::~API()
{
    if(_pThread)
        delete _pThread;
}

void API::start()
{
    if(_pThread)
        return;

    _pThread = new std::thread(&API::thread, this);
}

void API::stop()
{
    _bShutdown = true;
    while(!_bShutdownDone)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void API::thread()
{
    _bShutdownDone = false;

    //rpc_io_service = new asio::io_service();
    //rpc_ssl_context = new ssl::context(*rpc_io_service, ssl::context::sslv23);

    while(!_bShutdown)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    _bShutdownDone = true;
}
