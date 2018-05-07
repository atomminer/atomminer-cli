#ifndef GZ_H
#define GZ_H

#include <QByteArray>
#include <zlib.h>

#define GZIP_WINDOWS_BIT 15 + 16
#define GZIP_CHUNK_SIZE 32 * 1024

class gz
{
public:
    static bool get(QByteArray input, QByteArray &output);
};

#endif // GZ_H
