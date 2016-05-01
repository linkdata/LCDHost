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

#ifndef RAWINPUTENGINE_H
#define RAWINPUTENGINE_H

#include <QMutex>
#include <QList>

#include "AppRawInput.h"
#include "EventRawInput.h"

class RawInputDevice;

class RawInputEngine
{
protected:
    QMutex listmutex_;
    AppRawInput *app_rawinput;
    QList<RawInputDevice*> devicelist_;

public:
    RawInputEngine( AppRawInput *p ) : app_rawinput(p) {}
    virtual ~RawInputEngine();

    AppRawInput *appRawInput() const { return app_rawinput; }

    virtual void clear();
    virtual void scan() {}
    QMutex* listMutex() { return &listmutex_; }
    QList<RawInputDevice*> list() { return devicelist_; }
};

#endif // RAWINPUTENGINE_H
