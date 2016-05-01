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
#include <QDir>
#include <QCoreApplication>
#include <QStringBuilder>

#include "LCDHost.h"
#include "AppState.h"
#include "MainWindow.h"
#include "LibObject.h"
#include "LibLibrary.h"
#include "AppLibrary.h"
#include "LibClass.h"
#include "LibInstance.h"
#include "LibDevice.h"
#include "LibDeviceData.h"

#include "EventClassRemoved.h"
#include "EventClassLoaded.h"
#include "EventDeviceCreate.h"
#include "EventDeviceChanged.h"
#include "EventDeviceOpen.h"
#include "EventDeviceClose.h"
#include "EventLayoutChanged.h"
#include "EventNotify.h"
#include "EventCreateInstance.h"
#include "EventDestroyInstance.h"
#include "EventClearTimeout.h"
#include "EventLibraryLoaded.h"
#include "EventRequestVisibility.h"
#include "EventCallback.h"
#include "EventObjectCreated.h"
#include "EventRender.h"
#include "EventLayoutLoad.h"


LibLibrary::LibLibrary( QLibrary *lib, AppId id ) :
    LibObject(id, AppLibrary::simplenameFromFileInfo( QFileInfo(lib->fileName()) ),0,0), lib_(lib)
{
    Q_ASSERT( lib_ && lib_->isLoaded() );
    objtable_ = 0;
    memset( &table_, 0, sizeof(table_) );
}

LibLibrary::~LibLibrary()
{
    memset( &table_, 0, sizeof(table_) );
    objtable_ = 0;
}

void LibLibrary::loadComplete()
{
    id().postAppEvent( new EventLibraryLoaded() );
    notify(0,0);
    return;
}

QString LibLibrary::init()
{
    QString s;
    const char *retv = NULL;

    table_.lh_create = (void * (*) (lh_callback_t, void *)) lib_->resolve("lh_create");
    if( !table_.lh_create ) return tr("missing lh_create()");

    table_.lh_get_object_calltable = (const lh_object_calltable *(*) (void*)) lib_->resolve("lh_get_object_calltable");
    if( !table_.lh_get_object_calltable ) return tr("missing lh_get_object_calltable()");

    table_.lh_destroy = (void (*) (void *)) lib_->resolve("lh_destroy");
    if( !table_.lh_destroy ) return tr("missing lh_destroy()");

    setObj( table_.lh_create( lh_callback, this ) );
    if( obj() == 0 ) return tr("lh_create() failed");

    objtable_ = table_.lh_get_object_calltable( obj() );
    if( objtable_ == 0 ) return tr("lh_get_object_calltable returned NULL");
    if( objtable_->size != sizeof(lh_object_calltable) )
    {
        objtable_ = 0;
        return tr("lh_object_calltable is wrong size");
    }

    s = LibObject::init();
    if( !s.isEmpty() ) return s;
    if( retv ) return QString(retv);
    return QString();
}

void LibLibrary::term()
{
    if( id().isValid() )
    {
        foreach( QObject *o, children() )
        {
            LibObject *lib_obj = qobject_cast<LibObject*>(o);
            if( lib_obj )
            {
                lib_obj->term();
                delete lib_obj;
            }
        }

        LibObject::term();

        if( table_.lh_destroy ) table_.lh_destroy(obj());
        memset( &table_, 0, sizeof(table_) );
        objtable_ = 0;
    }
    else
        qWarning() << "LibLibrary::term()" << metaObject()->className() << objectName() << "has no id";

    return;
}

LibObject *LibLibrary::childFromId(AppId id)
{
    for( QList<QObject*>::const_iterator i = children().constBegin(); i != children().constEnd(); ++i )
    {
        LibObject *o = qobject_cast<LibObject*>(*i);
        if( o && o->id() == id ) return o;
    }
    return NULL;
}

LibObject *LibLibrary::childFromObj(const void *obj)
{
    for( QList<QObject*>::const_iterator i = children().constBegin(); i != children().constEnd(); ++i )
    {
        LibObject *o = qobject_cast<LibObject*>(*i);
        if( o && o->obj() == obj ) return o;
    }
    return NULL;
}

int LibLibrary::notify(int code, void *param)
{
    if( code & LH_NOTE_SECOND )
    {
        QCoreApplication::postEvent( mainWindow, new EventClearTimeout( objectName() ) );
        // qDebug("LibLibrary::notify(): %s QObject count: %d", qPrintable(objectName()), (findChildren<QObject *>()).size());
    }

    return LH_NOTE_SECOND | LibObject::notify(code,param);
}

// Forward LCDHost events to the recipients. We need to do this from the libraries,
// because walking the children() array of an object that belongs to another thread
// is unsafe, so resolving an AppId to an object can't be done until we've switched
// to the receiving thread.

bool LibLibrary::event( QEvent *event )
{
    if( LibObject::event(event) ) return true;
    Q_ASSERT( id().isValid() );
    Q_ASSERT( id().hasLibObject() );

    if( event->type() == EventCreateInstance::type() )
    {
        LibClass *lib_class;
        LibInstance *lib_inst;
        EventCreateInstance *e = static_cast<EventCreateInstance*>(event);

        if( lh_log_load )
            qDebug() << QString("<span style=\"background-color: #f0f0f0;\"><tt>%1</tt></span> Create instance %2 of type %3")
                       .arg(id().toString())
                       .arg(e->inst_id.toString())
                       .arg(e->class_id)
                        ;

        lib_class = getClassById( e->class_id );
        if( lib_class && lib_class->isValid() )
        {
            const void *obj = lib_class->obj_new();
            if( obj )
            {
                QString retv;
                lib_inst = new LibInstance( e->inst_id, *lib_class, e->obj_name, obj, this );
                retv = lib_inst->init();
                if( !retv.isEmpty() )
                {
                    qWarning() << e->class_id << ":" << retv;
                    delete lib_inst;
                    lib_inst = 0;

                }
            }
            else
                qWarning() << "LibLibrary::event()::EventCreateInstance fails: obj_new() returned NULL for" << e->class_id;
        }
        else
            qWarning() << "LibLibrary::event()::EventCreateInstance can't instantiate " << e->class_id;
        return true;
    }

    if( event->type() == EventRender::type() )
    {
        EventRender *e = static_cast<EventRender*>(event);
        LibDevice *lib_dev = qobject_cast<LibDevice*>(childFromId(e->id));
        if( lib_dev ) lib_dev->render(e->image);
        return true;
    }

    if( event->type() == EventDeviceCreate::type() )
    {
        EventDeviceCreate *e = static_cast<EventDeviceCreate*>(event);
        LibDevice *lib_dev = new LibDevice( e->id, e->devid, this );
        QString retv = lib_dev->init();
        if( !retv.isEmpty() )
        {
            qWarning() << "EventDeviceCreate:" << retv;
            lib_dev->term();
            delete lib_dev;
        }
        return true;
    }

    if( event->type() == EventDeviceOpen::type() )
    {
        Q_ASSERT(0); // Should go to LibDevice::event()
        return true;
    }

    if( event->type() == EventDeviceClose::type() )
    {
        EventDeviceClose *e = static_cast<EventDeviceClose*>(event);
        LibDevice *lib_dev = qobject_cast<LibDevice*>(childFromId(e->id));
        if( lib_dev ) lib_dev->close();
        return true;
    }

    if( event->type() == EventDestroyInstance::type() )
    {
        EventDestroyInstance *e = static_cast<EventDestroyInstance*>(event);
        LibInstance *lib_inst = qobject_cast<LibInstance*>(childFromId(e->id));
        if( lib_inst )
        {
            lib_inst->term();
            delete lib_inst;
        }
        return true;
    }

    qWarning() << "LibLibrary::event() unhandled user event" << EventBase::name(event->type()) << "for" << objectName();
    return false;
}
