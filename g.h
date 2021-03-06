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

#ifndef G_H
#define G_H

#include <QTime>

#include "firmware/firmware.h"
#include "device/hotplug.h"
#include "device/devicemanager.h"

class G
{
public:
    G();

    static volatile bool isShuttingDown;
    static DeviceManager DevMgr;
    static QDateTime    startTime;
    static Firmware     fw;

    // TODO: Stratum manager?

    static HotPlug *HotPlugHandler;
};

#endif // G_H
