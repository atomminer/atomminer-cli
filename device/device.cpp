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

#include "device.h"

#define DEFAULT_CMD_TIMEOUT 200

AtomMinerDevice::AtomMinerDevice(libusb_device *dev)
{
    _handle = NULL;
    _dev = dev;

    libusb_device_descriptor desc;

    libusb_get_device_descriptor(dev, &desc);
    int rc = libusb_open(dev, &_handle);
    if (LIBUSB_SUCCESS != rc)
    {
        _dev = NULL;
        _handle = NULL;
        return;
    }
    unsigned char s[256];

    VID = desc.idVendor;
    PID = desc.idProduct;
    s[0] = 0;
    if(desc.iManufacturer > 0)
        libusb_get_string_descriptor_ascii(_handle, desc.iManufacturer, s, 256);
    Manufacturer = (const char*)s;

    s[0] = 0;
    if(desc.iProduct > 0)
        libusb_get_string_descriptor_ascii(_handle, desc.iProduct, s, 256);
    Product = (const char*)s;

    s[0] = 0;
    if(desc.iSerialNumber > 0)
        libusb_get_string_descriptor_ascii(_handle, desc.iSerialNumber, s, 256);
    Serial = (const char*)s;

    GetFirmwareInfo();
}

bool AtomMinerDevice::isValid()
{
    return (_dev != NULL);
}

bool AtomMinerDevice::isConnected()
{
    return _handle != NULL;
}

bool AtomMinerDevice::Open()
{
    if(!isValid())
        return false;
    if(!_handle)
    {
        int rc = libusb_open(_dev, &_handle);
        if (LIBUSB_SUCCESS != rc)
        {
            _handle = NULL;
            return false;
        }
    }
    return true;
}

void AtomMinerDevice::Close()
{
    if(_handle)
    {
        libusb_close(_handle);
        _handle = NULL;
    }
}

void AtomMinerDevice::Reset()
{
    libusb_reset_device(_handle);
}

bool AtomMinerDevice::Reconnect()
{
    Close();
    return Open();
}

void AtomMinerDevice::BlinkLED()
{
    libusb_control_transfer(_handle, 0x77, 0, 0x02, 0, 0, 0, DEFAULT_CMD_TIMEOUT);
}

int AtomMinerDevice::sendData(uint8_t channel, uint8_t *buf, uint32_t size)
{
    int transferred = 0;
    libusb_bulk_transfer(_handle, channel, buf, size, &transferred, DEFAULT_CMD_TIMEOUT);
    return transferred;
}

void AtomMinerDevice::sendCmd(uint8_t reqType, uint8_t req, uint16_t value, uint16_t idx)
{
    libusb_control_transfer(_handle, reqType, req, value, idx, 0, 0, DEFAULT_CMD_TIMEOUT);
}

int AtomMinerDevice::readCmd(uint8_t reqType, uint8_t req, uint16_t value, uint16_t idx, uint8_t *buf, uint32_t size)
{
    return libusb_control_transfer(_handle, reqType, req, value, idx, buf, size, DEFAULT_CMD_TIMEOUT);
}

int AtomMinerDevice::readData(uint8_t channel, uint8_t *buf, uint32_t size)
{
    int transferred = 0;
    libusb_bulk_transfer(_handle, channel, buf, size, &transferred, 100);
    return transferred;
}

std::string AtomMinerDevice::DeviceString()
{
    std::string s = "";

    if(!isValid())
        return s;

    s += "[" + Product + "] " + Manufacturer;
    s += "\r\n";
    s += Serial;

    return s;
}

std::string AtomMinerDevice::DeviceStringFormatForList()
{
    char s[512];
    sprintf(s, "<html><head/><body><p>%s %s</p><p align=\"right\">%s</p></body></html>", Product.c_str(), Manufacturer.c_str(), Serial.c_str());

    return s;
}

std::string AtomMinerDevice::GetFirmwareInfo()
{
    uint8_t buf[64];
    int r = libusb_control_transfer(_handle, 0xb2, 0x01, 0, 0, buf, 64, DEFAULT_CMD_TIMEOUT);
    if(r != 64)
        FirmwareVersion = "";
    else
    {
        FirmwareVersion = std::string((const char*)buf + 8);
    }

    return FirmwareVersion;
}

libusb_device *AtomMinerDevice::getLL()
{
    return _dev;
}

libusb_device_handle *AtomMinerDevice::handle()
{
    return _handle;
}


// Internal use commands. NOT FOR PUBLIC RELEASE
