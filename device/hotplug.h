/*
 * AtomMiner Device Lib
 * Copyright AtomMiner, 2018,
 * All Rights Reserved
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF ATOMMINER
 *
 *      Author: AtomMiner - null3128
 */

#ifndef HOTPLUG_H
#define HOTPLUG_H

#include <QThread>
#include <QStringList>
#include <QTime>
#include <QDebug>
#include <QQueue>

#include "device/device.h"


class HotPlug : public QThread
{
    Q_OBJECT
public:
    HotPlug();

    void run();

    static QQueue<libusb_device*>   newDevices;
    static QQueue<libusb_device*>   goneDevices;

    bool bActive;

signals:
    void deviceAdd(libusb_device*);
    void deviceGone(libusb_device*);
};

#endif // HOTPLUG_H
