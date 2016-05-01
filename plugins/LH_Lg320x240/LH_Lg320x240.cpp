/**
  \file     LH_Lg320x240.cpp
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

#include <QtGlobal>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QCoreApplication>
#include <QTimerEvent>
#include <QThread>

#ifdef Q_OS_WIN
# include <windows.h>
#endif

#include "LH_QtDevice.h"
#include "LH_Lg320x240.h"
#include "LH_Qt_QImage.h"
#include "LogitechG19.h"

LH_PLUGIN(LH_Lg320x240)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Lg320x240</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" "r" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Johan \"SirReal\" Lindh</author>"
  "<homepageurl><a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Logitech 320x240 LCD via USB"
  "</shortdesc>"
  "<longdesc>"
    "USB-level driver for Logitech 320x240 LCD displays, such as the G19.<br/>"
    "On Windows, you'll need a <a href=\"http://en.wikipedia.org/wiki/WinUSB\">WinUSB</a> driver for your device.<br/>"
    "The easiest way to do that is to download <a href=\"http://sourceforge.net/projects/libwdi/files/zadig/zadig_v1.1.1.137.7z/download\">zadig</a>"
    "which can generate a driver for your G19."
  "</longdesc>"
"</lcdhostplugin>";

extern "C"
{
    void libusb_log( const char *fmt, va_list args )
    {
        char buf[1024];
        vsprintf( buf, fmt, args );
        qDebug() << buf;
    }
}

LH_Lg320x240::LH_Lg320x240() :
    LH_QtPlugin(),
    timer_id_(0),
    usb_ctx_(0),
    usb_device_list_(0)
{
}

const char *LH_Lg320x240::userInit()
{
#ifdef Q_OS_WIN
    // make sure neither LCDMon.exe nor LCORE.EXE is running on Windows
    if( FindWindowA( "Logitech LCD Monitor Window", "LCDMon" ) ||
        FindWindowA( "QWidget", "LCore" ) )
        return "Logitech drivers are loaded";
#endif
    if(!usb_ctx_)
    {
        if(libusb_init(&usb_ctx_) || !usb_ctx_)
            return "libusb_init() failed";
        libusb_set_debug(usb_ctx_, 1);
    }

    if(!timer_id_)
        timer_id_ = startTimer(2000);

    return NULL;
}

void LH_Lg320x240::userTerm()
{
    if(timer_id_)
    {
        killTimer(timer_id_);
        timer_id_ = 0;
    }
    if(usb_device_list_)
        qCritical("LH_Lg320x240::userTerm(): device enumeration in progress");
    if(usb_ctx_)
    {
        libusb_exit(usb_ctx_);
        usb_ctx_ = 0;
    }
}

void LH_Lg320x240::timerEvent(QTimerEvent *ev)
{
    if(usb_ctx_ && !usb_device_list_ && ev->timerId() == timer_id_)
    {
        LogitechG19 *g19 = findChild<LogitechG19 *>();

        if(g19)
            return;

        int usb_device_count = libusb_get_device_list(usb_ctx_, &usb_device_list_);
        if(usb_device_list_)
        {
            struct libusb_device_descriptor dd;
            memset(&dd, 0, sizeof(dd));
            for(int i = 0; i < usb_device_count; ++i)
            {
                if(usb_device_list_[i])
                {
                    if (!g19 &&
                            !libusb_get_device_descriptor(usb_device_list_[i], &dd) &&
                            dd.idVendor == 0x046d &&
                            dd.idProduct == 0xc229)
                    {
                        g19 = new LogitechG19(usb_ctx_, usb_device_list_[i], &dd, this);
                    }
                    else
                    {
                        libusb_unref_device(usb_device_list_[i]);
                    }
                    usb_device_list_[i] = 0;
                }
            }
            libusb_free_device_list(usb_device_list_, 0);
            usb_device_list_ = 0;
        }
    }
}
