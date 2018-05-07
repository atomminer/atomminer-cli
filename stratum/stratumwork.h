#ifndef STRATUMJOB_H
#define STRATUMJOB_H

#include <QString>
#include <QVector>

#define NONCE2_MAX_SIZE 16

struct StratumWork
{
    QString algo;
    QString jobID;
    QString ntime;
    QString xnonce2;
    QString nonce;

    QByteArray header;
    QByteArray target;
};

class StratumJob
{
public:
    StratumJob();
    ~StratumJob();

    // returns new block header for mining every time being called
    StratumWork* getNewWork();

    void construct();

protected:
    QByteArray reverse(QByteArray arr);

public:
    QString _algo;
    QString _jobID;
    QString _prevhash;
    QString _coinb1;
    QString _coinb2;
    QString _version;
    QString _nbits;
    QString _ntime;
    double _diff;
    QVector<QString> _merkle;

    QString _xnonce;
    uint32_t _xnonceSize;

    uint8_t _xnonce2[NONCE2_MAX_SIZE]; // max supported extranonce2
    int     _xnonce2Size;
    uint8_t *_coinbase;
};

#endif // STRATUMJOB_H
