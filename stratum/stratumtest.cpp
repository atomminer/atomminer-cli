#include <QThread>
#include "utils/log.h"
#include "stratumtest.h"

StratumTest::StratumTest()
{
    ;
}

bool StratumTest::_connect()
{
    _bConnected = true;
    _bDisconnect = false;
    _curl = (CURL*)123;

    sendNewJobCounter = 0;
    maxNewJobs = 1;

    _buffer = "{\"id\":null,\"method\":\"mining.set_difficulty\",\"params\":[1]}\n";
    _buffer += "{\"id\":1,\"result\":[[[\"mining.set_difficulty\",\"1\"],[\"mining.notify\",\"7fa4c8ae33a5393144c6f942349432f6\"]],\"81003514\",4],\"error\":null}\n";
    //setupTestBuffer();

    return true;
}

void StratumTest::setupTestBuffer()
{
//    {\"id\":1,\"result\":[[[\"mining.set_difficulty\",\"1\"],[\"mining.notify\",\"7fa4c8ae33a5393144c6f942349432f6\"]],\"81003514\",4],\"error\":null}
//    {\"id\":null,\"method\":\"mining.notify\",\"params\":[\"273c\",\"115530008e3d626e0e8c44bac998b6dcca8957f00b0f55ee0001eacb00000000\",\"02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff1f03644e26062f503253482f04461bee5a08\",\"7969696d700000000000010008af2f000000001976a91457043d950601c3fe063d70906e25dfca6dc85b1e88ac00000000\",[],\"00000070\",\"1b061550\",\"5aee1b45\",true]}
//
//      Block Header
//    70000000003055116e623d8eba448c0edcb698c9f05789caee550f0bcbea010000000000 0926c075ff12a304c2134af09aef9e19c270db9e38a73129e420ed88fc142b26 451bee5a5015061b00000000


    if(sendNewJobCounter >= maxNewJobs)
    { // just to keep 'conn' alive
        _buffer += "{\"id\":null,\"method\":\"mining.ping\",\"params\":[]}\n";
        return ;
    }
    sendNewJobCounter++;

    _buffer += "{\"id\":null,\"method\":\"mining.notify\",\"params\":[\"273c\",\"115530008e3d626e0e8c44bac998b6dcca8957f00b0f55ee0001eacb00000000\",\"02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff1f03644e26062f503253482f04461bee5a08\",\"7969696d700000000000010008af2f000000001976a91457043d950601c3fe063d70906e25dfca6dc85b1e88ac00000000\",[],\"00000070\",\"1b061550\",\"5aee1b45\",true]}\n";
}

void StratumTest::_disconnect(std::string reason)
{
    ;
}
bool StratumTest::_socket_full(int timeout)
{
    if(timeout == 300)
    {
        QThread::sleep(5); // 5 seconds is
        setupTestBuffer();
    }
    return true;
}

bool StratumTest::_send(std::string s)
{
    return true;
}

std::string StratumTest::_recv()
{
    std::string str = "";
    int n;

    if(-1 != (n = _buffer.find("\n")))
    {
        str = _buffer.substr(0, n + 1);
        _buffer.erase(0, n + 1);
        logproto(str);
        return str;
    }

    return "";
}

bool StratumTest::_subscribe()
{
    return true;
}
bool StratumTest::_authorize()
{
    return true;
}
bool StratumTest::_authorize_atom()
{
    return false;
}
