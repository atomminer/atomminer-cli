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

#ifndef __ATOM_DEVICE_HEADER_H__
#define __ATOM_DEVICE_HEADER_H__
#include <string>

#include <libusb.h>

#define MAX_FWIMG_SIZE		(512 * 1024)	// Maximum size of the firmware binary.

class AtomMinerDevice
{
public:
    AtomMinerDevice(libusb_device *dev);

    bool isValid();
    bool isConnected();
    bool open();
    void close();

    void reset();
    bool reconnect();

    void blinkLED();

    bool getIO(uint8_t *IObuf);

    int sendData(uint8_t channel, uint8_t *buf, uint32_t size);
    void sendCmd(uint8_t reqType, uint8_t req, uint16_t value, uint16_t idx, char* data = nullptr, int len = 0);
    int readCmd(uint8_t reqType, uint8_t req, uint16_t value, uint16_t idx, uint8_t *buf, uint32_t size);

    int readData(uint8_t channel, uint8_t *buf, uint32_t size);

    std::string DeviceString();
    std::string DeviceStringFormatForList();
    std::string firmwareInfo();

    libusb_device *getLL();
    libusb_device_handle *handle();

private:

public:
    std::string Manufacturer;
    std::string Product;
    std::string Serial;

    std::string FirmwareVersion;

    uint32_t VID;
    uint32_t PID;

private:
    libusb_device           *_dev;
    libusb_device_handle    *_handle;


public: // Internal use commands. NOT FOR PUBLIC RELEASE

};

#endif
