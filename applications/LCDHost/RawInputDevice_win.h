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

#ifndef RAWINPUTDEVICE_WIN_H
#define RAWINPUTDEVICE_WIN_H

#include <QtGlobal>
#include <QDateTime>
#include <QVector>
#include "RawInputDevice.h"

#ifdef Q_OS_WIN
# ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0501 // 0x0501 WinXP, 0x0601 WinVista
# endif
# include <windows.h>
extern "C" {
# if defined(__MINGW32__)
#  if (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 7))
#   include <ddk/hidsdi.h>
#  else
#   include <hidsdi.h>
#  endif
# define HID_CAPCOUNT_T ULONG
# else
#  if defined(Q_OS_WIN64)
#   include <hidsdi.h>
#   define HID_CAPCOUNT_T USHORT
#  else
#   include <api/hidsdi.h>
#   define HID_CAPCOUNT_T USHORT
#  endif
# endif
}
#endif

class AppRawInput;
class RawInputEngine_win;

class RawInputDevice_win : public RawInputDevice
{
public:
    RawInputDevice_win( RawInputEngine_win *e, QByteArray devicename, HANDLE h );
    ~RawInputDevice_win();

    QString itemName( int item, int val, int flags );
    bool setCapture( bool f );

    HANDLE handle() const { return handle_; }
    unsigned vendor() const { return vendor_; }
    unsigned product() const { return product_; }
    unsigned version() const { return devinfo_.dwType == RIM_TYPEHID ? devinfo_.hid.dwVersionNumber : 0; }
    unsigned page() const { return devcaps_.UsagePage; }
    unsigned usage() const { return devcaps_.Usage; }
    PHIDP_PREPARSED_DATA preparsed() const { return preparsed_; }

    void process( RAWINPUT *ri );
    void process_Usages( RAWINPUT *ri );
    void process_Values( RAWINPUT *ri );
    void process_Logitech_G19( RAWINPUT *ri );
    void process_Logitech_G15v1( RAWINPUT *ri );
    void process_Logitech_G15v2( RAWINPUT *ri );
    void process_Logitech_G13( RAWINPUT *ri );
    void process_Logitech_G11( RAWINPUT *ri );

    static RawInputDevice_win* getDevice( HANDLE h );

private:
    static QVector<RawInputDevice_win*> vector_;

    void parseName(QString devicename);

    HANDLE handle_; // The RawInput handle
    unsigned vendor_;
    unsigned product_;
    RID_DEVICE_INFO devinfo_;
    PHIDP_PREPARSED_DATA preparsed_;
    HIDP_CAPS devcaps_;
    ULONG usagemax_;
    ULONG prevusagelen_;
    QVector<USAGE> usagepages_;
    void *prevdata_; // previous raw HID data
    int prevsize_; // previous raw HID data size
    ULONG **prevvalue_; // ULONG [ devcaps_.NumberInputValueCaps ] [ ... ]
    USAGE **currusagelist_;  // USAGE [ usagepages_.size() ][ usagemax_ ]
    USAGE **prevusagelist_;  // USAGE [ usagepages_.size() ][ usagemax_ ]
    HIDP_BUTTON_CAPS *buttoncaps_; // HIDP_BUTTON_CAPS [ devcaps_.NumberInputButtonCaps ]
    HIDP_VALUE_CAPS *valuecaps_; // HIDP_VALUE_CAPS [ devcaps_.NumberInputValueCaps ]
    LONG prevmousex_;
    LONG prevmousey_;
};


#endif // RAWINPUTDEVICE_WIN_H
