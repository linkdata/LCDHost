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

#ifndef LIBDEVICE_H
#define LIBDEVICE_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QSize>
#include <QImage>

#include "LibObject.h"
#include "LibLibrary.h"
#include "LibDeviceData.h"

class LibDevice : public LibObject
{
    Q_OBJECT

    // static QMutex devmutex_;
    // static QList<lh_device*> devlist_;

    QByteArray devid_;
    QString buttonname_[LH_DEVICE_MAXBUTTONS];
    int buttonstate_;

public:
    LibDevice( AppId id, QByteArray devid, LibLibrary *parent );

    virtual QString init();
    virtual void term();

    LibLibrary *drv() const { return static_cast<LibLibrary*>(parent()); }
//    const QByteArray& devId() const { return devid_; }

    lh_device *lockptr();
    virtual const lh_object_calltable *lock();
    virtual const lh_device_calltable *devlock();
    virtual void unlock();

    const char* open();
    void render(QImage img);
    bool close();

    bool event( QEvent *event );

    static void arrive(AppId parent_id, const void *obj, lh_device *dev );
    static void leave(AppId parent_id, const void *obj, lh_device *dev );
};

#endif // LIBDEVICE_H
