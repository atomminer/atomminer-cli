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

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTcpServer>
#include <QString>
#include <QSsl>
#include <QSslSocket>
#include <QSslCertificate>
#include <QSslKey>
#include <QThread>

#include "api/api.h"
#include "g.h"
#include "settings/conf.h"
#include "stratum/stratum.h"
#include "utils/log.h"
#include "utils/fmt/format.h"

APIServer::APIServer(QObject *parent) : QTcpServer(parent)
{
    _useSSL = false;

    if(!conf()->apiSSL())
        return;

    QFile certificateFile(conf()->apiCert());

    if(!certificateFile.open(QIODevice::ReadOnly))
    {
        loge("Can't read api certificate file. Reverting back to non-secure connection");
        return;
    }

    auto data = certificateFile.readAll();

    _key = QSslKey(data, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, conf()->apiCertKeyPass().toLatin1());
    _cert = QSslCertificate(data, QSsl::Pem);

    _useSSL = true;
}


void APIServer::incomingConnection(qintptr socketDescriptor)
{
    // Create socket
    QSslSocket *sslSocket = new QSslSocket(this);
    sslSocket->setSocketDescriptor(socketDescriptor);

    if(conf()->apiSSL() && _useSSL)
    {
        sslSocket->setLocalCertificate(_cert);
        sslSocket->setPrivateKey(_key);
        sslSocket->setProtocol(QSsl::TlsV1SslV3);
        sslSocket->startServerEncryption();

        sslSocket->waitForEncrypted(1000);
    }

    this->addPendingConnection(sslSocket);
}

bool APIServer::checkAuth(QString sRequest)
{
    foreach(QString line, sRequest.split('\n')) {
        int colon = line.indexOf(':');
        QString headerName = line.left(colon).trimmed();
        QString headerValue = line.mid(colon + 1).trimmed();

        if(headerName == "Authorization")
        {
            if(headerValue.left(6) == "Basic ")
            {
                QString sLoginPass = QByteArray::fromBase64(headerValue.mid(6).toLatin1());
                return (sLoginPass == (conf()->apiUser() + ":" + conf()->apiPassword()));
            }
        }
    }
    return false;
}

QString APIServer::requestPath(QString sRequest)
{
    QStringList tokens = sRequest.split(QRegExp("[ \r\n][ \r\n]*"));
    if(tokens.at(0) == "GET" || tokens.at(0) == "POST")
    {
         QStringList path = tokens.at(1).split("?");
         return path.at(0);
    }

    return "";
}

QString APIServer::buildReplyHeaders(int code, QString reply)
{
    QString sStatus = "OK";
    if(code == 403)
        sStatus = "Forbidden";
    else if(code == 404)
        sStatus = "Not Found";

    QString s = QString("HTTP/1.1 %1 ").arg(code) + sStatus +"\r\n";
    s += "Connection: close\r\n";
    s += "Content-Type: application/json\r\n";
    s += QString("Content-Length: %1\r\n").arg(reply.length());
    s += "\r\n";
    s += reply;

    return s;
}

int APIServer::buildResponse(QString request, QString *response)
{
    if(!checkAuth(request))
    {
        *response ="403 - Not authorized";
        return 403;
    }

    QString sPath = "";
    sPath = requestPath(request);
    if(sPath.isEmpty())
        sPath = "/";

    if(sPath == "/" || sPath == "/stats")
    {
        *response = stats();
        return 200;
    }
    else if(sPath.indexOf("/dev/") == 0)
    {
        *response = deviceStats(sPath.mid(5));
        return 200;
    }

    *response ="Not found";
    return 404;
}

QString APIServer::stats()
{
    QJsonObject root;

    root.insert("stratum", QString::fromStdString(stratum()->getStatus()));
    root.insert("api", "ok");
    root.insert("uptime", G::startTime.secsTo(QDateTime::currentDateTime()));

    QJsonArray algos;
    root.insert("algos", algos);

    QJsonArray miners;
    root.insert("devices", miners);

    return QJsonDocument(root).toJson();
}

QString APIServer::deviceStats(QString device)
{
    QJsonObject root;
    root.insert("status", "not implemented");

    return QJsonDocument(root).toJson();
}

void APIServer::run()
{
    QHostAddress address = QHostAddress::Any;
    quint16 port = (quint16)conf()->apiPort();

    if (listen(address, port))
    {
        if(!_useSSL)
            logw(QString("API is listening on port %1").arg(port).toStdString());
        else
            logw(QString("API is listening on port %1 with SSL").arg(port).toStdString());
    }
    else
    {
        loge(QString("Failed to listen on port %1").arg(port).toStdString());
        loge("API will not be available");
        return;
    }

    while(!G::isShuttingDown)
    {
        if (waitForNewConnection(100))
        {
            QSslSocket *sslSocket = dynamic_cast<QSslSocket*>(nextPendingConnection());

            if(_useSSL && !sslSocket->isEncrypted())
            {
                sslSocket->disconnectFromHost();
                sslSocket->deleteLater();
                continue;
            }

            QString sReq = "";
            if (sslSocket->waitForReadyRead(100))
                sReq = sslSocket->readAll();

            QString response;
            int code = buildResponse(sReq, &response);

            sslSocket->write(buildReplyHeaders(code, response).toUtf8());
            sslSocket->waitForBytesWritten();

            sslSocket->disconnectFromHost();
            sslSocket->deleteLater();
        }
    }

    this->deleteLater();
}


static APIServer *gpAPIInstance = nullptr;

void apiStart()
{
    if(gpAPIInstance)
        return;

    if(!gpAPIInstance)
    {
        QThread *thread = new QThread;
        gpAPIInstance = new APIServer;

        gpAPIInstance->moveToThread(thread);
        QObject::connect(thread, SIGNAL(started()), gpAPIInstance, SLOT(run()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }
}
