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

#include "hotplug.h"
#include <libusb.h>
#include <mutex>
#include <sstream>

#include "g.h"

#define ATOM_VID    0x04b4

std::mutex  g_hotplug_event_sync_mutex;

// http://libusb.sourceforge.net/api-1.0/hotplug.html
int hotplug_callback(struct libusb_context *ctx __attribute__((unused)), struct libusb_device *dev,
                     libusb_hotplug_event event, void *user_data __attribute__((unused)))
{
  if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event)
  {
      std::lock_guard<std::mutex> lock(g_hotplug_event_sync_mutex);
      HotPlug::newDevices.enqueue(dev);
  }
  else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event)
  {
      std::lock_guard<std::mutex> lock(g_hotplug_event_sync_mutex);
      HotPlug::goneDevices.enqueue(dev);
  }

  return 0;
}

QQueue<libusb_device*>   HotPlug::newDevices;
QQueue<libusb_device*>   HotPlug::goneDevices;

HotPlug::HotPlug()
{
    bActive = false;
}

void HotPlug::run()
{
    libusb_hotplug_callback_handle handle;

    libusb_init(NULL);
    libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)3,
                                            LIBUSB_HOTPLUG_ENUMERATE, ATOM_VID, LIBUSB_HOTPLUG_MATCH_ANY,
                                            LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, NULL,
                                            &handle);

    bActive = true;

    while(!G::isShuttingDown)
    {
        struct timeval tv = {0 , 100};
        libusb_handle_events_timeout_completed(NULL, &tv, NULL);

        std::lock_guard<std::mutex> lock(g_hotplug_event_sync_mutex);
        if(goneDevices.size())
        {
            libusb_device *dev = goneDevices.dequeue();
            G::DevMgr.remove(dev);
            emit deviceGone(dev);
        }

        if(newDevices.size())
        {
            libusb_device *d = newDevices.dequeue();
            AtomMinerDevice *dev = new AtomMinerDevice(d);
            G::DevMgr.add(d, dev);
            emit deviceAdd(d);
        }
        QThread::msleep(50);
    }

    libusb_hotplug_deregister_callback(NULL, handle);

    bActive = false;
}
