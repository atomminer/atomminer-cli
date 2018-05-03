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

#ifndef LOG_H
#define LOG_H

void log(std::string s);
void loge(std::string s);   // error
void logw(std::string s);   // warning
void logi(std::string s);   // info
void logd(std::string s);   // debug
void logn(std::string s);   // notice

void logproto(std::string s);

void logprogress(std::string s);
void logprogressK(std::string s);
void logprogressF(std::string s);

#endif // LOG_H
