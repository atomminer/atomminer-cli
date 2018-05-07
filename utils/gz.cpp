#include <QByteArray>
#include "gz.h"

bool gz::get(QByteArray input, QByteArray &output)
{
    output.clear();

    if(!input.length())
        return true;

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    int ret = inflateInit2(&strm, GZIP_WINDOWS_BIT);

    if (ret != Z_OK)
        return(false);

    char *input_data = input.data();
    int input_data_left = input.length();

    do
    {
        int chunk_size = qMin(GZIP_CHUNK_SIZE, input_data_left);

        if(chunk_size <= 0)
            break;

        strm.next_in = (unsigned char*)input_data;
        strm.avail_in = chunk_size;
        input_data += chunk_size;
        input_data_left -= chunk_size;

        do
        {
            char out[GZIP_CHUNK_SIZE];
            strm.next_out = (unsigned char*)out;
            strm.avail_out = GZIP_CHUNK_SIZE;
            ret = inflate(&strm, Z_NO_FLUSH);

            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
                break;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
            case Z_STREAM_ERROR:
                inflateEnd(&strm);

                return(false);
            }

            int have = (GZIP_CHUNK_SIZE - strm.avail_out);

            if(have > 0)
                output.append((char*)out, have);

        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    return (ret == Z_STREAM_END);
}
