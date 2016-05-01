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

#include <QtDebug>
#include <QCoreApplication>
#include <QString>

#include "LCDHost.h"
#include "MainWindow.h"
#include "LibDriver.h"
#include "LibDevice.h"
#include "AppLibrary.h"
#include "EventLibraryLoaded.h"
#include "EventSetupItemRefresh.h"
#include "EventObjectCreated.h"
#include "EventNotify.h"
#include "EventRender.h"
#include "EventDeviceOpen.h"
#include "EventDeviceClose.h"
#include "EventClearTimeout.h"
#include "EventCreateDevice.h"
#include "EventCreateAppDevice.h"
#include "EventDestroyDevice.h"
#include "EventDestroyAppDevice.h"
#include "EventDriverEnumerate.h"
#include "lh_plugin.h"
#include "../lh_driver.h"

extern "C" void lh_driver_callback( void *id, const void *obj, lh_callbackcode code, void *param )
{
    LibDriver *drv = reinterpret_cast<LibDriver *>(id);
    lh_device *dev = NULL;
    if( drv == NULL ) return;

    switch( code )
    {
    case lh_cb_unload:
        Q_ASSERT(0);  // TODO
        break;

    case lh_cb_log:
        if( param ) qDebug() << drv->id() << ": " << QString::fromUtf8( (const char *) param );
        break;

    case lh_cb_arrive:
        drv->mutex().lock();
        dev = (lh_device*)param;
        new LibDevice( QString::fromUtf8(dev->id), drv, obj, dev );
        drv->mutex().unlock();
        break;

    case lh_cb_leave:
        drv->deleteFromObj(obj);
        break;

    case lh_cb_setup_refresh:
        drv->postAppEvent(obj,new EventSetupItemRefresh(AppSetupItem(static_cast<lh_setup_item*>(param))));
        break;
    case lh_cb_setup_rebuild:
        Q_ASSERT(0);  // TODO
        break;
    case lh_cb_button:
        Q_ASSERT(0);  // TODO
        break;
    case lh_cb_polling:
    case lh_cb_notify:
    case lh_cb_render:
    case lh_cb_unused:
        break;
    }

    return;
}

void LibDriver::callback( const void *obj, lh_callbackcode code, void *param )
{
    lh_driver_callback( this, obj, code, param );
    return;
}

LibDriver::LibDriver( AppId id, QFileInfo fi ) : LibLibrary(id,fi)
{
    memset( &table_, 0, sizeof(table_) );
    notify_ = ~0;
    return;
}

// Load the DLL, get function adresses
bool LibDriver::load()
{
    const char *msg = NULL;

    Q_ASSERT( !isLoaded() );
    if( !LibLibrary::load() )
    {
        qWarning() << fileName() << "failed to load";
        return false;
    }

    // driver-specific, optional
    table_.lh_notify = (int(*)(int,void*)) resolve("lh_notify");

    // Looks OK
    if( table_.lh_load ) msg = table_.lh_load( this, lh_driver_callback, &systemstate_ );
    if( msg == NULL )
    {
        loadComplete();
        return true;
    }
    else
        qWarning() << fileName() << "load() failed :" << msg;

    return false;
}

// Unload DLL, zero adresses
bool LibDriver::unload()
{
    if( table_.lh_unload ) table_.lh_unload();
    memset( &table_, 0, sizeof(table_) );
    return LibLibrary::unload();
}

bool LibDriver::event( QEvent *event )
{
    QCoreApplication::postEvent( mainWindow, new EventClearTimeout(id()) );

    if( event->type() == EventCreateAppDevice::type() )
    {
        EventCreateAppDevice *e = static_cast<EventCreateAppDevice*>(event);
        foreach( QObject *o, children() )
        {
            LibDevice *dev = qobject_cast<LibDevice *>(o);
            if( dev && dev->devId() == e->devid && dev->id().isEmpty() )
            {
                dev->setId( e->id );
                postAppEvent( e->id, new EventObjectCreated() );
                return true;
            }
        }
        // not found, so nuke the AppDevice
        postAppEvent( e->id, new EventDestroyAppDevice(e->id,e->devid) );
        return true;
    }

    if( event->type() == EventNotify::type() )
    {
        EventNotify *e = static_cast<EventNotify*>(event);
        setNotifyFilter( notify(e->note,e->param) | LH_NOTE_SECOND );
        return true;
    }

    return LibLibrary::event( event );
}

int LibDriver::notify( int code, void*param )
{
    if( !(code & notify_) ) return notify_;
    if( table_.lh_notify )
        return notify_ = table_.lh_notify( code, param );
    return 0;
}
#endif
