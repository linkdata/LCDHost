/**
  \file     LogitechDevice.h
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

#ifndef LOGITECHDEVICE_H
#define LOGITECHDEVICE_H

#include "LH_LgLcdMan.h"
#include "LH_QtDevice.h"

#ifdef Q_OS_WIN
# ifndef UNICODE
#  error ("This isn't going to work")
# endif
# include "windows.h"
// # include "../wow64.h"
# include "win/lglcd.h"
#endif

#ifdef Q_OS_MAC
# include "mac/lgLcdError.h"
# include "mac/lgLcd.h"
# ifndef ERROR_FILE_NOT_FOUND
#  define ERROR_FILE_NOT_FOUND 2
# endif
#endif

class LogitechDevice : public LH_QtDevice
{
    Q_OBJECT
    bool opened_;
    bool bw_; // if true a BW device, else QVGA
    unsigned long buttonState_;
    LH_LgLcdMan *drv_;

public:
    LogitechDevice( LH_LgLcdMan *drv, bool bw );
    ~LogitechDevice();

    LH_LgLcdMan *drv() const { return drv_; }
    const char *open() { opened_ = true; return NULL; }
    const char* render_qimage(QImage*);
    int buttons() { return (int) buttonState_; }
    const char* get_backlight(lh_device_backlight*);
    const char* set_backlight(lh_device_backlight*);
    const char *close();

    bool opened() const { return opened_; }

    // LH_LgLcdMan *drv() const { return static_cast<LH_LgLcdMan *>(LH_QtDevice::drv()); }
    unsigned long buttonState() const { return buttonState_; }
    void setButtonState( unsigned long ul );
};

#endif // LOGITECHDEVICE_H
