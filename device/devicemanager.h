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

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <vector>
#include <map>
#include <mutex>
#include <libusb.h>
#include "device.h"

class DeviceManager
{
public:
    DeviceManager();
    ~DeviceManager();

    void add(libusb_device *usbdev);
    void add(libusb_device *usbdev, AtomMinerDevice *dev);
    void remove(libusb_device *usbdev);
    AtomMinerDevice *get(libusb_device *dev);
    AtomMinerDevice *get(uint32_t idx);
    uint32_t count();

    bool has(AtomMinerDevice *dev);

private:
    std::mutex                      _mutex;
    std::map<libusb_device*, AtomMinerDevice*> _devs;
};

#endif // DEVICEMANAGER_H
