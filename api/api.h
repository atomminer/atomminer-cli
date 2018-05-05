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

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QSslError>
#include <QSslKey>
#include <QSslSocket>
#include <QTcpServer>

class APIServer : public QTcpServer
{
Q_OBJECT
public:
    APIServer(QObject *parent = 0);

    void setSslLocalCertificate(const QSslCertificate &certificate);
    bool setSslLocalCertificate(const QString &path, QSsl::EncodingFormat format = QSsl::Pem);

    void setSslPrivateKey(const QSslKey &key);
    bool setSslPrivateKey(const QString &fileName, QSsl::KeyAlgorithm algorithm = QSsl::Rsa, QSsl::EncodingFormat format = QSsl::Pem, const QByteArray &passPhrase = QByteArray());

private:
    bool checkAuth(QString sRequest);
    QString requestPath(QString sRequest);
    QString buildReplyHeaders(int code, QString reply ="");
    int buildResponse(QString request, QString *response);

    QString stats();
    QString deviceStats(QString device);

private:
    QSslKey             _key;
    QSslCertificate     _cert;
    bool                _useSSL;

protected:
    void incomingConnection(qintptr socketDescriptor) override final;

public slots:
    void run();
};


void apiStart();

#endif
