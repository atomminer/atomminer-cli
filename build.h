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

#ifndef BUILD_H
#define BUILD_H

#include "utils/types.h"
#include "utils/log.h"

#define PACKAGE_NAME    "AtomMiner CLI"
#define PACKAGE_VERSION "1.0.0"

#define USER_AGENT      "atomminer-cli/" PACKAGE_VERSION

#define BUILD_DATE __DATE__ " " __TIME__ "\x0"

#define STRINGIFY(x) #x

#ifdef _MSC_VER
//MSVC++ 6.0   _MSC_VER == 1200
//MSVC++ 7.0   _MSC_VER == 1300
//MSVC++ 7.1   _MSC_VER == 1310
//MSVC++ 8.0   _MSC_VER == 1400
//MSVC++ 9.0   _MSC_VER == 1500
//MSVC++ 10.0  _MSC_VER == 1600
//MSVC++ 11.0  _MSC_VER == 1700
//MSVC++ 12.0  _MSC_VER == 1800
//MSVC++ 14.0  _MSC_VER == 1900 (Visual Studio 2015 version 14.0)
//MSVC++ 14.1  _MSC_VER == 1910 (Visual Studio 2017 version 15.0)
//MSVC++ 14.11 _MSC_VER == 1911 (Visual Studio 2017 version 15.3)
//MSVC++ 14.12 _MSC_VER == 1912 (Visual Studio 2017 version 15.5)
//MSVC++ 14.13 _MSC_VER == 1913 (Visual Studio 2017 version 15.6)
//MSVC++ 14.14 _MSC_VER == 1914 (Visual Studio 2017 version 15.7)
    #define COMPILER         "VC++ " _MSC_VER
#elif defined(__GNUC__)
    #define COMPILER         "GCC " __VERSION__
//STRINGIFY(__GNUC__) "." STRINGIFY(__GNUC_MINOR__) "." STRINGIFY(__GNUC_PATCHLEVEL__) "\x0"
#endif

#undef unlikely
#undef likely
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define unlikely(expr) (__builtin_expect(!!(expr), 0))
#define likely(expr) (__builtin_expect(!!(expr), 1))
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

#endif // BUILD_H
