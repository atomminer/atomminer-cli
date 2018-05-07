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

#include <algorithm>
#include "devicemanager.h"

DeviceManager::DeviceManager()
{
}

DeviceManager::~DeviceManager()
{
    std::lock_guard<std::mutex> lock(_mutex);
    for (auto const& it : _devs)
        delete it.second;

    _devs.erase(_devs.begin(), _devs.end());
}

void DeviceManager::add(libusb_device *usbdev)
{
    AtomMinerDevice *dev = new AtomMinerDevice(usbdev);
    add(usbdev, dev);
}

void DeviceManager::add(libusb_device *usbdev, AtomMinerDevice *dev)
{
    if(!dev)
        return;

    std::lock_guard<std::mutex> lock(_mutex);
    _devs[usbdev] = dev;
}

void DeviceManager::remove(libusb_device *dev)
{
    if(!dev)
        return;

    std::lock_guard<std::mutex> lock(_mutex);

    AtomMinerDevice *adev = _devs[dev];
    _devs.erase(dev);
    if(adev)
        delete adev;
}

uint32_t DeviceManager::count()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _devs.size();
}

AtomMinerDevice *DeviceManager::get(libusb_device *dev)
{
    std::lock_guard<std::mutex> lock(_mutex);

    return _devs[dev];
}

AtomMinerDevice *DeviceManager::get(uint32_t idx)
{
    std::lock_guard<std::mutex> lock(_mutex);

    std::map<libusb_device*, AtomMinerDevice*>::iterator it( _devs.begin() );
    std::advance(it, idx);
    return it->second;
}

bool DeviceManager::has(AtomMinerDevice *dev)
{
    std::lock_guard<std::mutex> lock(_mutex);

    for (const auto& it : _devs)
        if(it.second == dev)
            return true;

    return false;
}
