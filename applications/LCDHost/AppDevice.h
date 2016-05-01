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

#ifndef APPDEVICE_H
#define APPDEVICE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QSize>
#include <QImage>
#include <QList>

#include "AppObject.h"
#include "AppLibrary.h"
#include "LibDevice.h"
#include "EventDeviceOpen.h"
#include "EventDeviceClose.h"
#include "EventRender.h"

class AppDevice : public AppObject
{
    Q_OBJECT
    static AppDevice *current_;
    QByteArray devid_;
    QString name_;
    QSize size_;
    int depth_;
    QImage logo_;
    bool autoselect_;
    // int present_;

public:
    AppDevice();
    AppDevice( AppLibrary *drv, QByteArray devid, QString name, QSize size, int depth, bool autoselect, QImage logo );
    ~AppDevice();
    
    // two stage create/destroy
    virtual void init();
    virtual void term();

    AppLibrary *appLibrary() const { return static_cast<AppLibrary*>(parent()); }

    QByteArray devId() const { return devid_; }
    QString name() const { return name_; }
    QSize size() const { return size_; }
    int depth() const { return depth_; }
    QImage logo() const { return logo_; }

    bool isValid() const { return depth_ && parent() && !id().isEmpty(); }
    bool isMonochrome() const { return depth_==1; }
    bool isAutoselect() const { return autoselect_; }
    // bool isPresent() const { return present_ > 0; }
    // void libDeviceCreated();
    // void libDeviceDestroyed();

    void open() { id().postLibEvent( new EventDeviceOpen(id()) ); }
    void render( QImage img ) { id().postLibEvent( new EventRender(id(),img) ); }
    void close() { id().postLibEvent( new EventDeviceClose(id()) ); }

    static bool hasCurrent() { return current_ != NULL; }
    static AppDevice& current();
    static bool setCurrent( AppDevice *dev );
    static bool setCurrent( AppId id )
    {
        return setCurrent( qobject_cast<AppDevice*>(id.appObject()) );
    }

    static AppDevice* fromId( AppId id ) { return qobject_cast<AppDevice*>(id.appObject()); }
};

#endif // APPDEVICE_H
