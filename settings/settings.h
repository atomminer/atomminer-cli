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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <string>
#include "json/jansson.hpp"

class Conf
{
public:
    Conf();

    static Conf *inst();

    bool load(std::string path);

    std::string pool();
    std::string user();
    std::string password();

    bool api();
    int  apiPort();
    bool apiSSL();
    bool apiLogin();
    std::string apiCert();
    std::string apiUser();
    std::string apiPassword();

private:
    std::string _stringValue(std::string name, std::string def);
    bool _boolValue(std::string name, bool def);
    int _intValue(std::string name, int def);

private:
    static Conf *_instance;
    json::Value _config;

};

Conf *conf();

#endif
