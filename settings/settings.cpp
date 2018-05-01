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

#include <unistd.h>
#include <iostream>
#include <fstream>
#include "json/jansson.hpp"
#include "utils/log.h"
#include "settings.h"

Conf *Conf::_instance = nullptr;
static const char *default_settings = "{\"pool\":\"http://keccak.pool.atomminer.com:5133\",\"user\":\"\",\"password\":\"\",\"api\":true,\"api_port\":9123,\"api_ssl\":true,\"api_cert\":\"\",\"api_login\":true,\"api_user\":\"api\",\"api_password\":\"pass\"}";

Conf *conf()
{
    return Conf::inst();
}

Conf *Conf::inst()
{
    if(!Conf::_instance)
        Conf::_instance = new Conf();

    return _instance;
}

Conf::Conf()
{
    ;
}

bool Conf::load(std::string path)
{
    if(path.length() > 0) {
        std::ifstream f(path.c_str());
        if(!f.good())
            return false;
        f.close();
    }

    json::json_error_t error;

    if(path.length() > 0)
        _config = json::load_file(path.c_str(), &error);
    else
        _config = json::load_string(default_settings, &error);

    if(!_config)
        return false;

    return true;
}

std::string Conf::_stringValue(std::string name, std::string def)
{
    if(!_config[name.c_str()])
        return def;
    return _config[name.c_str()].as_string();
}

bool Conf::_boolValue(std::string name, bool def)
{
    if(!_config[name.c_str()])
        return def;
    return _config[name.c_str()].as_boolean();
}

int Conf::_intValue(std::string name, int def)
{
    if(!_config[name.c_str()])
        return def;
    return _config[name.c_str()].as_integer();
}

std::string Conf::pool()
{
    return _stringValue("pool", "http://keccak.pool.atomminer.com:5133");
}

std::string Conf::user()
{
    return _stringValue("user", "");
}

std::string Conf::password()
{
    return _stringValue("password", "");
}

bool Conf::api()
{
    return _boolValue("api", true);
}

int  Conf::apiPort()
{
    return _intValue("api_port", 9123);
}

bool Conf::apiSSL()
{
    return _boolValue("api_ssl", true);
}

bool Conf::apiLogin()
{
    return _boolValue("api_login", false);
}
std::string Conf::apiCert()
{
    return _stringValue("api_cert", "");
}

std::string Conf::apiUser()
{
    return _stringValue("api_user", "api");
}

std::string Conf::apiPassword()
{
    return _stringValue("api_password", "pass");
}

