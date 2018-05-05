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

#include "g.h"

G::G()
{

}

volatile bool G::isShuttingDown = false;
DeviceManager G::DevMgr;
QDateTime    G::startTime = QDateTime::currentDateTime();

HotPlug G::HotPlugHandler;
