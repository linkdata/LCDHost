/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

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

#ifndef LOGITECHG19_H
#define LOGITECHG19_H

#include <QDateTime>
#include <LH_QtDevice.h>
extern "C" {
#include "libusb.h"
}


class LogitechG19 : public LH_QtDevice
{
    Q_OBJECT

    static int g19_event_;

    libusb_context *usb_ctx_;
    libusb_device *usbdev_;
    libusb_device_handle *lcdhandle_;
    int lcd_if_number_;
    int menukeys_if_number_;
    uint8_t endpoint_in_;
    uint8_t endpoint_out_;
    libusb_transfer * button_transfer_;
    int button_completed_;
    int last_buttons_;
    volatile quint16 new_buttons_;

public:
    static void LIBUSB_CALL g19_button_cb( struct libusb_transfer * transfer );
    LogitechG19(libusb_context *ctx, libusb_device *usbdev, libusb_device_descriptor *dd, LH_QtObject *parent = 0);
    ~LogitechG19();

    libusb_device *usb_device() const { return usbdev_; }
    void userTerm();
    void customEvent(QEvent *ev);

    const char* render_argb32(int,int,const void*) { return NULL; }
    const char* render_mono(int,int,const void*) { return NULL; }
    const char* get_backlight(lh_device_backlight*) { return NULL; }
    const char* set_backlight(lh_device_backlight*) { return NULL; }

    void buttons( struct libusb_transfer * transfer = 0 );
    bool offline() const;

    const char* open();
    const char* close();
    const char* render_qimage(QImage *img);

};

#endif // LOGITECHG19_H
