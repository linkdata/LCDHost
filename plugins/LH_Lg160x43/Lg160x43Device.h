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

#ifndef LG160X43DEVICE_H
#define LG160X43DEVICE_H

#include "LH_QtDevice.h"

class LH_HidDevice;

class Lg160x43Device : public LH_QtDevice
{
    Q_OBJECT

    LH_HidDevice *hd_;
    unsigned char output_report_id_;

public:
    Lg160x43Device(LH_HidDevice *hi, int output_report_id, LH_QtPlugin *drv = 0);

    const char* open() { return NULL; }
    const char* render_argb32(int,int,const void*) { return NULL; }
    const char* render_mono(int,int,const void*) { return NULL; }
    int buttons() { return 0; }
    const char* get_backlight(lh_device_backlight*) { return NULL; }
    const char* set_backlight(lh_device_backlight*) { return NULL; }
    const char* close() { return NULL; }

    const char* render_qimage(QImage *img);

public slots:
    void onlineChanged(bool b);
};

#endif // LG160X43DEVICE_H
