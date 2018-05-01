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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "json/jansson.hpp"
#include "utils/fmt/format.h"
#include "stratum.h"
#include "build.h"
#include "g.h"

// utility funcs
#ifdef WIN32
#define socket_blocks() (WSAGetLastError() == WSAEWOULDBLOCK)
#else
#define socket_blocks() (errno == EAGAIN || errno == EWOULDBLOCK)
#endif

// static curl cb
static int curl_sockopt_keepalive_cb(void *, curl_socket_t fd, curlsocktype )
{
#ifdef __linux
    int tcp_keepcnt = 3;
#endif
    int tcp_keepintvl = 50;
    int tcp_keepidle = 50;
#ifndef WIN32
    int keepalive = 1;
    if (unlikely(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive,
        sizeof(keepalive))))
        return 1;
#ifdef __linux
    if (unlikely(setsockopt(fd, SOL_TCP, TCP_KEEPCNT,
        &tcp_keepcnt, sizeof(tcp_keepcnt))))
        return 1;
    if (unlikely(setsockopt(fd, SOL_TCP, TCP_KEEPIDLE,
        &tcp_keepidle, sizeof(tcp_keepidle))))
        return 1;
    if (unlikely(setsockopt(fd, SOL_TCP, TCP_KEEPINTVL,
        &tcp_keepintvl, sizeof(tcp_keepintvl))))
        return 1;
#endif /* __linux */
#ifdef __APPLE_CC__
    if (unlikely(setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE,
        &tcp_keepintvl, sizeof(tcp_keepintvl))))
        return 1;
#endif /* __APPLE_CC__ */
#else /* WIN32 */
    struct tcp_keepalive vals;
    vals.onoff = 1;
    vals.keepalivetime = tcp_keepidle * 1000;
    vals.keepaliveinterval = tcp_keepintvl * 1000;
    DWORD outputBytes;
    if (unlikely(WSAIoctl(fd, SIO_KEEPALIVE_VALS, &vals, sizeof(vals),
        NULL, 0, &outputBytes, NULL, NULL)))
        return 1;
#endif /* WIN32 */

    return 0;
}

static curl_socket_t curl_opensocket_grab_cb(void *clientp, curlsocktype, struct curl_sockaddr *addr)
{
    curl_socket_t *sock = (curl_socket_t*) clientp;
    *sock = socket(addr->family, addr->socktype, addr->protocol);
    return *sock;
}


// Stratum class

Stratum *Stratum::_instance = nullptr;

Stratum* stratum()
{
    return Stratum::inst();
}

Stratum* Stratum::inst()
{
    if(!Stratum::_instance)
        Stratum::_instance = new Stratum();
    return _instance;
}

Stratum::Stratum()
{
    _url = "pool.atomminer.com:5133";
    _login = "";
    _pass = "x";
    _status = "Disconnected";

    _reconnect_wait_time = 10; // in seconds

    _bShutdown = false;
    _bConnected = false;
    _bDisconnect = false;
    _bConnect = false;

    _curl = nullptr;

    _pThread = new std::thread(&Stratum::thread, this);
}

Stratum::~Stratum()
{
    _pThread->join();
}


std::string Stratum::getStatus()
{
    lock_t lock(_mutex);

    return _status;
}

Stratum* Stratum::connect(std::string url, std::string login, std::string pass)
{
    lock_t lock(_mutex);

    _status = "Connecting";
    _url = url;
    _login = login;
    _pass = pass;

    _bConnected = false;
    _bDisconnect = true;
    _bConnect = true;

    return this;
}

Stratum* Stratum::disconnect()
{
    lock_t lock(_mutex);

    _bDisconnect = true;    // diconnect from pool
    _bConnect = false;      // do not allow to reconnect

    return this;
}

bool Stratum::isConnected()
{
    return _bConnected;
}

void Stratum::setAlgo(std::string algo)
{
    _algo = algo;
}

void Stratum::_disconnect(std::string reason)
{
    lock_t lock(_mutex);

    if (_curl) {
        curl_easy_cleanup(_curl);
        _curl = nullptr;
    }
    _bDisconnect = false;
    _bConnected = false;

    auto it = _jobs.begin();
    while (it != _jobs.end())
    {
        delete it->second;
        it++;
    }
    _jobs.clear();

    emit disconnected(reason);
}

bool Stratum::_connect()
{
    lock_t lock(_mutex);

    // reset what we have to
    _extraNonceSize = 0;
    _extraNonce = "";

    logd("Stratum::_connect()");

    _curl = curl_easy_init();
    if(!_curl)
    {
        loge("CURL initialization failed");
        _curl = nullptr;
        return false;
    }

    _buffer = "";
    _csrf = "123";

    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
    curl_easy_setopt(_curl, CURLOPT_FRESH_CONNECT, 1);
    curl_easy_setopt(_curl, CURLOPT_CONNECTTIMEOUT, 30);
    curl_easy_setopt(_curl, CURLOPT_ERRORBUFFER, _curlError);
    curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(_curl, CURLOPT_TCP_NODELAY, 1);

    // TODO:
    //curl_easy_setopt(_curl, CURLOPT_PROXY, opt_proxy);
    //curl_easy_setopt(_curl, CURLOPT_PROXYTYPE, opt_proxy_type);
    curl_easy_setopt(_curl, CURLOPT_HTTPPROXYTUNNEL, 1);
    curl_easy_setopt(_curl, CURLOPT_SOCKOPTFUNCTION, curl_sockopt_keepalive_cb);
    curl_easy_setopt(_curl, CURLOPT_OPENSOCKETFUNCTION, curl_opensocket_grab_cb);
    curl_easy_setopt(_curl, CURLOPT_OPENSOCKETDATA, &_sock);
    curl_easy_setopt(_curl, CURLOPT_CONNECT_ONLY, 1);
    int rc = curl_easy_perform(_curl);
    if (rc) {
        loge(fmt::format("Stratum connection failed: %s", _curlError));
        curl_easy_cleanup(_curl);
        _curl = nullptr;
        return false;
    }

    return true;
}

bool Stratum::_socket_full(int timeout)
{
    timeval tv;
    fd_set rd;

    FD_ZERO(&rd);
    FD_SET(_sock, &rd);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    if (select((int)(_sock + 1), &rd, NULL, NULL, &tv) > 0)
        return true;
    return false;
}

bool Stratum::_send(std::string sLine)
{
    size_t sent = 0;
    int len;

    logproto("> " + sLine);

    if(sLine[(sLine.length()-1)] != '\n')
        sLine += "\n";

    len = sLine.length();
    while (len > 0)
    {
        struct timeval timeout = {0, 0};
        int n;
        fd_set wd;

        FD_ZERO(&wd);
        FD_SET(_sock, &wd);
        if (select((int) (_sock + 1), NULL, &wd, NULL, &timeout) < 1)
            return false;
        n = send(_sock, sLine.c_str() + sent, len, 0);
        if (n < 0)
        {
            if (!socket_blocks())
                return false;
            n = 0;
        }
        sent += n;
        len -= n;
    }

    return true;
}

std::string Stratum::_recv()
{
    std::string str = "";
    int n;

    if(-1 != (n = _buffer.find("\n")))
    {
        str = _buffer.substr(0, n + 1);
        _buffer.erase(0, n + 1);
        return str;
    }
    { // try to receive it
        time_t rstart;
        time(&rstart);
        if (!_socket_full(60)) {
            loge("Stratum _recv timed out");
            return "";
        }
        do {
            char s[2048];
            ssize_t n;

            memset(s, 0, 2048);
            n = recv(_sock, s, 2048, 0);
            if (!n) {
                return "";
            }
            if (n < 0) {
                if (!socket_blocks() || !_socket_full(1)) {
                    return "";
                }
            } else
                str += s;
        } while (time(NULL) - rstart < 60 && -1 == (int)str.find("\n"));
    }

    logproto("< " + str);

    _buffer += str;

    return _recv();
}

bool Stratum::_subscribe()
{
    logd("Stratum::_subscribe()");
    std::string s = "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": [\"" USER_AGENT "\"]}";
    if(!_send(s))
    {
        logw("Stratum subscribe failed");
        return false;
    }

    if(!_socket_full(30))
    {
        logw("Stratum subscribe timed out");
        return false;
    }

    std::string str = _recv();

    if(str.length() == 0)
    {
        logw("No response from stratum");
        return false;
    }

    json::json_error_t jerror;
    json::Value json = json::load_string(str.c_str(), &jerror);

    if(!json)
    {
        logw(fmt::format("Invalid command: {}", str));
        return false;
    }

    auto res = json["result"];
    auto err = json["error"];
    if(res.is_undefined() || res.is_null() || (!err.is_null()))
    {
        logw("Wrong subscribe response");
        return false;
    }

    if(res.size() < 3)
    {
        logw("Wrong subscribe response");
        return false;
    }

    if((res[0][1])[0].as_string() == "mining.notify")
        _session = (res[0][1])[1].as_string();

    _extraNonce = res[1].as_string();
    _extraNonceSize = res[2].as_integer(); // extranonce size

    log(fmt::format("Stratum session: {0}", _session));

    if((res[0][0])[0].as_string() == "mining.set_difficulty")
    {
        _diff = (res[0][0])[1].as_real();
        log(fmt::format("Stratum set difficulty to {0}", _diff));
    }


    return true;
}

bool Stratum::_authorize()
{
    std::string s = fmt::format("{{\"id\": 3, \"method\": \"mining.authorize\", \"params\": [\"{}\", \"{}\", \"{}\"]}}", _login, _pass, _csrf);
    if(!_send(s))
    {
        logw("Error sending authorization");
        return false;
    }

    std::string str = _recv();

    if(str.length() == 0)
    {
        logw("No response from stratum");
        return false;
    }

    json::json_error_t jerror;
    auto json = json::load_string(str.c_str(), &jerror);

    if(!json)
    {
        logw(fmt::format("Invalid command: {}", str));
        return false;
    }

    // expected response:
    // {"id":2,"result":true,"error":null}
    auto res = json["result"];
    auto err = json["error"];
    if(res.is_undefined() || !res.is_boolean() || !err.is_null())
    {
        if(!err.is_null())
            logw(fmt::format("Authorization failed: {}", err[1].as_string()));
        else
            logw(fmt::format("Wrong authorization response: {}", str));
        return false;
    }

    return res.as_boolean();
}

bool Stratum::_authorize_atom()
{
    if(_csrf.length() == 0)
        return false;

    std::string s =
            fmt::format("{{\"id\": 2, \"method\": \"mining.authorize_atom\", \"params\": [\"{}\"]}}",
                        _csrf);
    if(!_send(s))
    {
        logw("Error sending authorization");
        return false;
    }

    std::string str = _recv();

    if(str.length() == 0)
    {
        logw("No response from stratum");
        return false;
    }

    json::json_error_t jerror;
    auto json = json::load_string(str.c_str(), &jerror);

    if(!json)
    {
        logw(fmt::format("Invalid command: {}", str));
        return false;
    }

    // expected response:
    // {"id":2,"result":true,"error":null}
    auto res = json["result"];
    auto err = json["error"];
    if(res.is_undefined() || !res.is_boolean() || !err.is_null())
    {
        if(!err.is_null())
            logw(fmt::format("Atom Authorization failed: {}", err[1].as_string()));
        else
            logw(fmt::format("Wrong authorization response: {}", str));
        return false;
    }

    return res.as_boolean();
}

void Stratum::_process_command(std::string cmd)
{
    lock_t lock(_mutex);
    if(cmd.length() == 0)
        return;

    json::json_error_t jerror;
    auto json = json::load_string(cmd.c_str(), &jerror);

    if(!json)
    {
        logw(fmt::format("Invalid command: {}", cmd));
        return;
    }

    int id = !json["id"].is_null() ? json["id"].as_integer() : 0;
    std::string method = !json["method"].is_null() ? json["method"].as_string() : "";
    auto params = json["params"];
    auto jalgo = json["algo"];

    if(method.length() == 0)
    {
        logw(fmt::format("Invalid command: {}", cmd));
        return;
    }

    if(method == "mining.notify")
    {
        int idx = 0;
        StratumJob *job = new StratumJob();
        //_merkle.clear();
        job->_jobID = params[idx++].as_string();
        job->_prevhash = params[idx++].as_string();
        job->_coinb1 = params[idx++].as_string();
        job->_coinb2 = params[idx++].as_string();
        // merkle here
        auto merkles = params[idx++];
        if(merkles.is_array())
        {
            for(int i = 0 ; i < (int)merkles.size() ; i++)
                job->_merkle.push_back(merkles[i].as_string());
        }
        job->_version = params[idx++].as_string();
        job->_nbits = params[idx++].as_string();
        job->_ntime = params[idx++].as_string();
        // we're ignoring restart flag here and will restart every time stratum sends notify

        std::string algo = _algo;
        if(jalgo)
            algo = jalgo.as_string();

        if(_jobs[algo])
            delete _jobs[algo]; // delete old job
        _jobs[algo] = job;

        emit newJobReceived(algo);
    }
    else if(method == "mining.set_difficulty")
    {
        _diff = params[0].as_real();
        log(fmt::format("Stratum set difficulty to {0}", _diff));
    }
    else if(method == "mining.set_extranonce")
    {
        if(!params[0] || !params[1])
        {
            logw(fmt::format("Invalid mining.set_extranonce: {}", params.save_string()));
            return;
        }
        int xSize = params[1].as_integer();
        if(xSize < 2 || xSize > 16)
        {
            logw(fmt::format("Invalid extranonce size: {}", params[1].save_string()));
            return;
        }

        _extraNonce = params[0].as_string();
        _extraNonceSize = params[1].as_integer();

    }
    else if(method == "mining.ping")
    {
        _send(fmt::format("{{\"id\":{},\"result\":\"pong\",\"error\":null}}", id));
    }
    else if(method == "client.reconnect")
    {
        log("Stratum requested reconnect");
        _disconnect("Stratum requested reconnect");
    }
    else if(method == "client.show_message")
    {
        log(params[0].as_string());
    }
    else
        logw(fmt::format("Unknown command from pool: {}", method));

}

void Stratum::thread()
{
    _bShutdownDone = false;
    while(!G::isShuttingDown)
    {
        if(_bDisconnect)
            _disconnect();

        if(!_curl && _bConnect)
        {
            if(!_connect())
            {
                log(fmt::format("Reconnect in %d seconds", _reconnect_wait_time));
                _disconnect("Error connecting");
                std::this_thread::sleep_for(std::chrono::milliseconds(_reconnect_wait_time * 1000));
                continue;
            }

            if(!_subscribe())
            { // we don't want to support stratum servers with no extranonce support!
                log("Cant' subscribe to stratum notifications. Reconnecting...");
                _disconnect("Failed to subscribe to stratum notifications");
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                continue;
            }

            if(!_authorize_atom())
            {
                logw("authorize_atom is recommended for better mining results. Reverting to authorize...");
                if(!_authorize())
                {
                    log("Authorization failed. Reconnecting...");
                    _disconnect("Authorization failed");
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    continue;
                }
            }

            _keepAliveTimer = std::chrono::system_clock::now();
            _bConnected = true;
            emit connected();
        }

        //read data and process
        if (!_socket_full(300)) // 5 minutes
        {
            loge("Connection timeout. Reconnecting...");
            _disconnect();
            continue;
        }

        auto s = _recv();
        if(s.length() == 0)
        {
            loge("Connection interrupted. Reconnecting...");
            _disconnect();
            continue;
        }

        _process_command(s);

        aclock_t timeNow = std::chrono::system_clock::now();
        long int seconds = std::chrono::duration_cast<std::chrono::seconds>(timeNow - _keepAliveTimer).count();

        if(seconds >= 120)
        { //2 minutes
            _keepAliveTimer = timeNow;
            _send("{\"id\":0,\"result\":\"pong\",\"error\":null}");
        }
    }

    _bShutdownDone = true;
}
