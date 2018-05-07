/*
 * AtomMiner util Lib
 * Copyright AtomMiner, 2018,
 * All Rights Reserved
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF ATOMMINER
 *
 *      Author: AtomMiner - null3128
 */

#include <QDir>
#include <QString>

#include "fmt/format.h"
#include "log.h"
#include "u.h"

U::U()
{

}

void U::bin(uint8_t *p, std::string str)
{
    const char *hexstr = str.c_str();
    size_t len = str.length();

    char hex_byte[3];
    char *ep;

    hex_byte[2] = '\0';

    while (*hexstr && len) {
        if (!hexstr[1]) {
            loge("binarize str truncated");
            return;
        }
        hex_byte[0] = hexstr[0];
        hex_byte[1] = hexstr[1];
        *p = (unsigned char) strtol(hex_byte, &ep, 16);
        if (*ep) {
            loge(fmt::format("binarize failed on '[]'", hex_byte));
            return ;
        }
        p++;
        hexstr += 2;
        len--;
    }
}

std::string U::hex(uint8_t *data, int len)
{
    std::string s = "";
    for (int i = 0; i < len; i++)
        s += fmt::format("{:2X}", data[i]);

    return s;
}

uint32_t U::le32dec(const void *pp)
{
    const uint8_t *p = (uint8_t const *)pp;
    return ((uint32_t)(p[0]) + ((uint32_t)(p[1]) << 8) +
        ((uint32_t)(p[2]) << 16) + ((uint32_t)(p[3]) << 24));
}

uint32_t U::le32(std::string s)
{
    uint32_t t;
    U::bin((uint8_t*)&t, s);
    return U::le32dec((const void*)&t);
}

uint32_t U::be32dec(const void *pp)
{
    const uint8_t *p = (uint8_t const *)pp;
    return ((uint32_t)(p[3]) + ((uint32_t)(p[2]) << 8) +
        ((uint32_t)(p[1]) << 16) + ((uint32_t)(p[0]) << 24));
}

void U::bswap32(char *data)
{
    char t = data[0];
    data[0] = data[3];
    data[3] = t;
    t = data[1];
    data[1] = data[2];
    data[2] = t;
}

QString U::home()
{
    return QDir::homePath();
}

QString U::firmware()
{
    return U::home() + "/.atomminer/firmware/";
}

QString U::files()
{
    return U::home() + "/.atomminer/";
}
