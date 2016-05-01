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
#include <QtGlobal>
#include <QCoreApplication>

#include "AppLibrary.h"
#include "EventRender.h"
#include "EventDeviceOpen.h"
#include "EventDeviceClose.h"
#include "LibLibrary.h"
#include "LibDevice.h"
#include "LibDevicePointer.h"
#include "EventCreateAppDevice.h"
#include "EventDestroyAppDevice.h"
#include "EventDeviceDestroy.h"

// QMutex LibDevice::devmutex_;
// QList<lh_device*> LibDevice::devlist_;

void LibDevice::arrive(AppId parent_id, const void *obj, lh_device* dev)
{
    Q_UNUSED(obj);
    Q_ASSERT(dev && dev->devid);

    if(dev && dev->devid)
    {
        LibDevicePointer *ldp = 0;
        QString device_id(QString::fromLatin1(dev->devid).replace('/', '-'));
        if(LibLibrary *parent_lib = static_cast<LibLibrary *>(parent_id.lock()))
        {
            ldp = parent_lib->findChild<LibDevicePointer *>(device_id);
            if(ldp == 0)
                new LibDevicePointer(dev, device_id, parent_lib);
            parent_id.unlock();
            if(ldp == 0)
            {
                parent_id.postAppEvent(new EventCreateAppDevice(
                                           QByteArray(dev->devid),
                                           QString::fromUtf8(dev->name),
                                           QSize(dev->width, dev->height),
                                           dev->depth,
                                           !dev->noauto
                                           ), Qt::HighEventPriority+1);
            }
        }
        else
            qCritical("LibDevice::arrive(%s): parent_id.lock() failed (%d)%s",
                      dev->devid, parent_id.toInt(), parent_id.hasLibObject() ? "" : " (no LibObject)");
    }

    return;
}


void LibDevice::leave(AppId parent_id, const void *obj, lh_device *dev )
{
    Q_UNUSED(obj);
    Q_ASSERT(dev && dev->devid);

    if(dev && dev->devid)
    {
        LibDevicePointer *ldp = 0;
        QString device_id(QString::fromLatin1(dev->devid).replace('/', '-'));
        if(LibLibrary *parent_lib = static_cast<LibLibrary *>(parent_id.lock()))
        {
            ldp = parent_lib->findChild<LibDevicePointer *>(device_id);
            if(ldp)
                ldp->clear();
            parent_id.unlock();
            if(ldp)
                parent_id.postAppEvent(new EventDestroyAppDevice(dev->devid), Qt::HighEventPriority+1);
        }
        else
            qCritical("LibDevice::leave(%s): parent_id.lock() failed (%d)%s",
                      dev->devid, parent_id.toInt(), parent_id.hasLibObject() ? "" : " (no LibObject)");
    }

    return;
}

LibDevice::LibDevice(AppId id, QByteArray devid, LibLibrary *parent) :
    LibObject(id, QString::fromLatin1(devid), 0, parent),
    devid_(devid),
    buttonstate_(0)
{
    return;
}

lh_device *LibDevice::lockptr()
{
    if(LibDevicePointer *ldp = drv()->findChild<LibDevicePointer *>(objectName()))
        return ldp->lock();
    return 0;
}

const lh_object_calltable *LibDevice::lock()
{
    if(lh_device *d = lockptr())
        return & d->objtable;
    return 0;
}

const lh_device_calltable *LibDevice::devlock()
{
    if(lh_device *d = lockptr())
        return & d->table;
    return 0;
}

void LibDevice::unlock()
{
    if(LibDevicePointer *ldp = drv()->findChild<LibDevicePointer *>(objectName()))
        ldp->unlock();
    // devmutex_.unlock();
}

QString LibDevice::init()
{
    Q_ASSERT( ! devid_.isEmpty() );
    Q_ASSERT( obj() == 0 );

    if(lh_device *dev = lockptr())
    {
        setObj(dev->obj);
        for( int i = 0; i < LH_DEVICE_MAXBUTTONS; ++i )
        {
            if(dev->button[i])
                buttonname_[i] = QString::fromUtf8(dev->button[i]);
        }
        unlock();
        return LibObject::init();
    }
    return tr("can't find device %1").arg(QString::fromUtf8(devid_));
}

void LibDevice::term()
{
    LibObject::term();
}

const char* LibDevice::open()
{
    const char *retv = NULL;
    if( const lh_device_calltable *devtable = devlock() )
    {
        retv = devtable->obj_open( obj() );
        unlock();
    }
    return retv;
}

void LibDevice::render(QImage img)
{
    const char * retv = 0;
    if( const lh_device_calltable *devtable = devlock() )
    {
        retv = devtable->obj_render_qimage( obj(), &img );
        unlock();
    }
    if( retv && *retv )
        qWarning( "%s: %s", devid_.constData(), retv );
    return;
}

bool LibDevice::close()
{
    const char *retv = 0;
    if( const lh_device_calltable *devtable = devlock() )
    {
        retv = devtable->obj_close( obj() );
        unlock();
    }
    if( retv )
    {
        qDebug() << objectName() << "failed to close:" << QString::fromLatin1(retv);
        return false;
    }
    return true;
}

bool LibDevice::event( QEvent *event )
{
    if( LibObject::event(event) ) return true;
    Q_ASSERT( event->type() >= QEvent::User );
    Q_ASSERT( event->type() <= QEvent::MaxUser );
    Q_ASSERT( id().isValid() );

    if( event->type() == EventDeviceOpen::type() )
    {
        if (const char* retv = open()) {
            qWarning("Failed to open device \"%s\": \"%s\"\n", name(), retv);
            term();
            deleteLater();
        }
        return true;
    }

    if( event->type() == EventDeviceClose::type() )
    {
        close();
        return true;
    }

    if( event->type() == EventRender::type() )
    {
        EventRender *e = static_cast<EventRender*>(event);
        render( e->image );
        return true;
    }

    if( event->type() == EventDeviceDestroy::type() )
    {
        close();
        if(LibDevicePointer *ldp = drv()->findChild<LibDevicePointer *>(objectName()))
            delete ldp;
        deleteLater();
        return true;
    }

    qWarning() << "LibDevice::event() unhandled user event" << EventBase::name(event->type()) << "for" << objectName();
    return false;
}
