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

#ifndef VIRTUALDEVICE_H
#define VIRTUALDEVICE_H

#include <LH_QtDevice.h>
#include <LH_Qt_QImage.h>

class VirtualDevice : public LH_QtDevice
{
    Q_OBJECT

protected:
    LH_Qt_QImage *setup_output_;

public:
    VirtualDevice(LH_QtPlugin *drv);

    const char* open();
    const char* render_argb32(int,int,const void*) { return NULL; }
    const char* render_mono(int,int,const void*) { return NULL; }
    int buttons() { return 0; }
    const char* get_backlight(lh_device_backlight*) { return NULL; }
    const char* set_backlight(lh_device_backlight*) { return NULL; }
    const char* close();
    const char* render_qimage(QImage *img);
};

#endif // VIRTUALDEVICE_H
