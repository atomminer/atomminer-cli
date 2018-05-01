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

#ifndef U_H
#define U_H

#include <string>

class U
{
public:
    U();

    // make sure dst is valid and can take the whole data in
    static void bin(uint8_t *dst, std::string s);
    static std::string hex(uint8_t *data, int len);
    static uint32_t le32dec(const void *pp);
    static uint32_t le32(std::string s);
    static uint32_t be32dec(const void *pp);
};

#endif // U_H
