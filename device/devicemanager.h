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

    void Add(libusb_device *usbdev);
    void Add(libusb_device *usbdev, AtomMinerDevice *dev);
    void Remove(libusb_device *usbdev);
    AtomMinerDevice *Get(libusb_device *dev);
    AtomMinerDevice *Get(uint32_t idx);
    uint32_t Count();

private:
    std::mutex                      _mutex;
    //std::vector<AtomMinerDevice*>   _devs;
    std::map<libusb_device*, AtomMinerDevice*> _devs;
};

#endif // DEVICEMANAGER_H
