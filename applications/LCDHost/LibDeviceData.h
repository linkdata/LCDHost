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

#ifndef LIBDEVICEDATA_H
#define LIBDEVICEDATA_H

#include <QString>
#include <QSize>
#include <QImage>
#include <QMutex>
#include "AppId.h"
#include "lh_plugin.h"

class LibDeviceData
{
    static LibDeviceData *first_;
    static QMutex mutex_;

    AppId id_;
    LibDeviceData *next_;
    const void *obj_;
    QString devid_;
    QString name_;
    QSize size_;
    int depth_;
    bool autoselect_;
    QImage logo_;
    QString buttonname_[LH_DEVICE_MAXBUTTONS];
    lh_object_calltable objtable_;
    lh_device_calltable table_;

    LibDeviceData( const void *obj, const lh_device *dev );

public:
    AppId id() const { return id_; }
    void setAppId( AppId id ) { id_ = id; }
    const void *obj() const { return obj_; }
    QString devId() const { return devid_; }
    QString name() const { return name_; }
    QSize size() const { return size_; }
    int depth() const { return depth_; }
    bool autoselect() const { return autoselect_; }
    QImage logo() const { return logo_; }
    const lh_device_calltable &table() const { return table_; }
    QString buttonName( int n ) const
    {
        if( n>=0 && n<LH_DEVICE_MAXBUTTONS ) return buttonname_[n];
        return QString();
    }

    bool operator==( const LibDeviceData& other ) const
    {
        return (other.devid_ == devid_);
    }

    static bool exists( QString devId );
    static LibDeviceData* add( const void *obj, lh_device *dev );
    static void remove( LibDeviceData* );

    static LibDeviceData* lockObj( const void *obj );
    static LibDeviceData* lock( QString devId );
    static void unlock();
};

#endif // LIBDEVICEDATA_H
