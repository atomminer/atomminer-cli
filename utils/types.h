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

#ifndef TYPES_H
#define TYPES_H
#include <chrono>
#include <mutex>

typedef std::lock_guard<std::mutex> lock_t;
typedef std::chrono::time_point<std::chrono::system_clock> aclock_t;

#endif // TYPES_H
