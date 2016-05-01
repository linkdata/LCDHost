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


#include <QDebug>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppDevice.h"
#include "EventDeviceCreate.h"
#include "EventDeviceDestroy.h"

AppDevice *AppDevice::current_ = NULL;

AppDevice::AppDevice() : AppObject(0)
{
    name_ = tr("(no device)");
    size_ = QSize(320,240);
    depth_ = 32;
    autoselect_ = true;
    return;
}

AppDevice::AppDevice( AppLibrary *drv, QByteArray devid, QString name, QSize size, int depth, bool autoselect, QImage logo ) : AppObject( drv )
{
    devid_ = devid;
    name_ = name;
    size_ = size;
    depth_ = depth;
    autoselect_ = autoselect;
    logo_ = logo;
    setObjectName(devid_);
    return;
}

AppDevice::~AppDevice()
{
    depth_ = 0;
    if( this == current_ ) current_ = NULL;
    return;
}

void AppDevice::init()
{
    AppObject::init();
    Q_ASSERT( id().hasAppObject() );
    return;
}

void AppDevice::term()
{
    if( id().isValid() )
    {
        id().postLibEvent( new EventDeviceDestroy( devId() ) );
        AppObject::term();
    }
    else
        qWarning() << "AppDevice::term()" << metaObject()->className() << objectName() << "not initialized";
}

AppDevice& AppDevice::current()
{
    static AppDevice nullDevice;
    if( current_ == NULL ) return nullDevice;
    return *current_;
}

bool AppDevice::setCurrent( AppDevice *app_device )
{
    if( app_device && !app_device->isValid() ) return false;
    if( current_ )
    {
        current_->close();
        current_ = NULL;
    }
    if( app_device )
    {
        Q_ASSERT( app_device->isValid() );
        current_ = app_device;
        current_->open();
    }
    return true;
}

