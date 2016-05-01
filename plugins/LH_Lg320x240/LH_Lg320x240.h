/**
  \file     LH_Lg320x240.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

  This file is part of LCDHost.

  LCDHost is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LCDHost is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with LCDHost.  If not, see <http://www.gnu.org/licenses/>.
  */

#ifndef LH_LG320x240_H
#define LH_LG320x240_H

#include <QEvent>
#include <LH_QtPlugin.h>

typedef struct libusb_context libusb_context;
typedef struct libusb_device libusb_device;
class LogitechG19;

class LH_Lg320x240 : public LH_QtPlugin
{
    Q_OBJECT

    int timer_id_;
    libusb_context *usb_ctx_;
    libusb_device **usb_device_list_;

public:
    LH_Lg320x240();
    const char *userInit();
    void userTerm();
    void timerEvent(QTimerEvent *ev);
};

#endif // LH_LG320x240_H
