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


#if 0
#include <QDebug>

#include "AppDriver.h"
#include "AppDevice.h"
#include "EventCreateAppDevice.h"
#include "EventDestroyAppDevice.h"
#include "EventDriverEnumerate.h"

// Represents a driver at the application level
// Each driver may expose zero or more devices.
// Each driver runs with the same separation as a plugin;
// meaning it's own thread and crash protections.

AppDriver::AppDriver(QFileInfo fi ) : AppLibrary( fi )
{
    type_ = Driver;
    connect( this, SIGNAL(stateChanged(AppId)), this, SLOT(stateChanged(AppId)) );
}

AppDriver::~AppDriver()
{
    disconnect();
    stop();
}

void AppDriver::stateChanged( AppId )
{
    AppDevice *dev;

    if( state() == AppLibrary::Failed )
    {
        foreach( QObject *o, children() )
        {
            dev = qobject_cast<AppDevice *>(o);
            Q_ASSERT( dev );
            while( dev && dev->isPresent() )
                dev->libDeviceDestroyed();
        }
    }
}

bool AppDriver::event( QEvent *event )
{
    AppDevice *dev;

    if( event->type() == EventCreateAppDevice::type() )
    {
        EventCreateAppDevice *e = static_cast<EventCreateAppDevice*>(event);
        foreach( QObject *o, children() )
        {
            dev = qobject_cast<AppDevice *>(o);
            Q_ASSERT( dev );
            if( dev && dev->devId() == e->devid )
            {
                dev->libDeviceCreated();
                return true;
            }
        }
        dev = new AppDevice(this,e->devid,e->name,e->size,e->depth,e->autoselect,e->logo);
        dev->setupComplete();
        return true;
    }

    if( event->type() == EventDestroyAppDevice::type() )
    {
        EventDestroyAppDevice *e = static_cast<EventDestroyAppDevice*>(event);
        foreach( QObject *o, children() )
        {
            dev = qobject_cast<AppDevice *>(o);
            Q_ASSERT( dev );
            if( dev && dev->devId() == e->devid )
            {
                dev->libDeviceDestroyed();
                return true;
            }
        }
        return true;
    }

    return AppLibrary::event(event);
}

#endif
