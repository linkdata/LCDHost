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

#include <QSettings>
#include <QBuffer>
#include <QDebug>
#include <QThread>
#include <QMap>
#include <QMutex>
#include <QApplication>
#include <QDir>

#include "LCDHost.h"
#include "MainWindow.h"
#include "EventCreateInstance.h"
#include "EventDestroyInstance.h"
#include "EventNotify.h"
#include "EventLayoutChanged.h"
#include "EventClassLoaded.h"
#include "EventClassRemoved.h"
#include "EventClearTimeout.h"
#include "EventCallback.h"
#include "EventDeviceChanged.h"
#include "EventLibraryLoaded.h"
#include "EventRequestRender.h"
#include "LibPlugin.h"
#include "LibClass.h"
#include "LibInstance.h"
#include "AppClass.h"
#include "AppPlugin.h"

/**
  Callback from the shared library. Don't process requests immediately,
  rather post them as events and return to the library.
  */
extern "C" void lh_plugin_callback( void *id, const void *obj, lh_callbackcode code, void *param )
{
    LibPlugin *plg = reinterpret_cast<LibPlugin *>(id);
    if( plg == NULL ) return;

    switch( code )
    {
    case lh_cb_render:
        plg->postAppEvent( obj, new EventRequestRender() );
        break;
    case lh_cb_setup_refresh:
    case lh_cb_setup_rebuild:
    case lh_cb_unload:
    case lh_cb_notify:
    case lh_cb_polling:
        plg->postLibEvent( obj, new EventCallback(obj,code,param) );
        break;
    case lh_cb_log:
        if( param ) qDebug() << plg->fileName() << QString::fromUtf8( (const char *) param );
        break;
    case lh_cb_arrive:
    case lh_cb_leave:
    case lh_cb_button:
    case lh_cb_unused:
        break;
    }

    return;
}


LibPlugin::LibPlugin( AppId id, QFileInfo fi ) : LibLibrary(id,fi)
{
    Q_ASSERT( !isLoaded() );
    memset( &table_, 0, sizeof(table_) );
    return;
}

LibPlugin::~LibPlugin()
{
    return;
}

LibClass *LibPlugin::getClassById( QString id ) const
{
    QList<LibClass*>::const_iterator ci;
    for( ci=classList.constBegin(); ci!=classList.constEnd(); ++ci )
        if( (*ci)->id() == id ) return (*ci);
    return NULL;
}

// Load the DLL, get function adresses
bool LibPlugin::load()
{
    const char *msg = NULL;

    Q_ASSERT( !isLoaded() );
    if( !LibLibrary::load() ) return false;

    // plugin-specific, but mandatory
    table_.lh_class_list = (const lh_class ** (*)(void)) resolve("lh_class_list");

    // Make sure we have all plugin mandatory
    if( table_.lh_class_list != NULL )
    {
        // Looks OK
        if( table().lh_load ) msg = table_.lh_load( this, lh_plugin_callback, &systemstate_ );

        if( msg == NULL )
        {
            loadClassList();
            loadComplete();
            return true;
        }
        else
            qWarning() << fileName() << "load() failed :" << msg;
    }
    else
        qWarning() << fileName() << "missing lh_class_list()";

    return false;
}

void LibPlugin::postLoad()
{
    foreach( LibClass *lib_class, classList )
    {
        postAppEvent( id(), new EventClassLoaded(
                lib_class->id(),
                lib_class->cls()->path ? QString::fromUtf8(lib_class->cls()->path) : QString(),
                QString::fromUtf8(lib_class->cls()->name),
                QSize( lib_class->cls()->width, lib_class->cls()->height )
                ) );
    }
    return;
}

// Unload DLL, zero adresses
bool LibPlugin::unload()
{
    clear();
    if( table_.lh_unload ) table_.lh_unload();
    memset( &table_, 0, sizeof(table_) );
    return LibLibrary::unload();
}

void LibPlugin::clear()
{
    LibClass *lib_class;

    foreach( QObject *o, children() )
    {
        Q_ASSERT( qobject_cast<LibInstance*>(o) );
        LibInstance *lib_inst = static_cast<LibInstance*>(o);
        if( lib_inst ) delete lib_inst;
    }

    while( !classList.isEmpty() )
    {
        lib_class = classList.takeFirst();
        postAppEvent( id(), new EventClassRemoved( lib_class->id() ) );
        delete lib_class;
    }

    return;
}

void LibPlugin::loadClassList()
{
    LibClass *lib_class;
    const lh_class **a_class_list;
    const lh_class *a_class;

    clear();

    a_class_list = lh_class_list();
    if( a_class_list )
    {
        a_class = *a_class_list;
        while( a_class )
        {
            if( a_class->size == sizeof(lh_class) && a_class->name )
            {
                lib_class = new LibClass( a_class, this );
                classList.append( lib_class );
                a_class_list++;
                a_class = *a_class_list;
            }
            else
            {
                qWarning() << fileName() << ": illegal class list entry";
                break;
            }
        }
    }

    return;
}

bool LibPlugin::event( QEvent *event )
{
    if( event->type() == EventDeviceChanged::type() )
    {
        EventDeviceChanged *e = static_cast<EventDeviceChanged*>(event);
        systemstate_.dev_width = e->size.width();
        systemstate_.dev_height = e->size.height();
        systemstate_.dev_depth = e->depth;
        systemstate_.dev_fps = 0;
        return true;
    }

    if( event->type() == EventLayoutChanged::type() )
    {
        EventLayoutChanged *e = static_cast<EventLayoutChanged*>(event);
        QByteArray name_utf8 = e->name.toUtf8();
        QByteArray dir_utf8 = e->dir.toUtf8();

        if( systemstate_.layout_file ) delete[] systemstate_.layout_file; systemstate_.layout_file = NULL;
        systemstate_.layout_file = new char[ name_utf8.size()+1 ];
        memcpy( systemstate_.layout_file, name_utf8.constData(), name_utf8.size()+1 );

        if( systemstate_.dir_layout ) delete[] systemstate_.dir_layout; systemstate_.dir_layout = NULL;
        systemstate_.dir_layout = new char[ dir_utf8.size()+1 ];
        memcpy( systemstate_.dir_layout, dir_utf8.constData(), dir_utf8.size()+1 );

        return true;
    }

    if( event->type() == EventNotify::type() )
    {
        EventNotify *e = static_cast<EventNotify*>(event);
        if( e->note & LH_NOTE_SECOND )
        {
            QCoreApplication::postEvent( mainWindow, new EventClearTimeout(id()) );
            systemstate_.dev_fps = mainWindow->systemState()->dev_fps;
            systemstate_.cpu_load = mainWindow->systemState()->cpu_load;
        }
        if( e->note & LH_NOTE_MEM )
        {
            memcpy( &systemstate_.mem_data, & mainWindow->systemState()->mem_data, sizeof(lh_memdata) );
        }
        if( e->note & LH_NOTE_NET )
        {
            systemstate_.net_max_in = mainWindow->systemState()->net_max_in;
            systemstate_.net_max_out = mainWindow->systemState()->net_max_out;
            memcpy( &systemstate_.net_data, & mainWindow->systemState()->net_data, sizeof(lh_netdata) );
        }
        if( e->note & LH_NOTE_CPU )
        {
            if( mainWindow->systemState()->cpu_count != systemstate_.cpu_count )
            {
                systemstate_.cpu_count = mainWindow->systemState()->cpu_count;
                if( systemstate_.cpu_cores ) delete[] systemstate_.cpu_cores;
                systemstate_.cpu_cores = new lh_cpudata[systemstate_.cpu_count];
            }
            if( systemstate_.cpu_cores )
                memcpy( systemstate_.cpu_cores, mainWindow->systemState()->cpu_cores, sizeof(lh_cpudata) * systemstate_.cpu_count );
        }
        foreach( QObject *o, children() )
        {
            Q_ASSERT( qobject_cast<LibInstance*>(o) );
            LibInstance *lib_inst = static_cast<LibInstance*>(o);
            if( lib_inst ) lib_inst->notify( e->note, e->param );
        }
    }

    if( event->type() == EventCreateInstance::type() )
    {
        LibClass *lib_class;
        LibInstance *lib_inst;
        EventCreateInstance *e = static_cast<EventCreateInstance*>(event);

        lib_class = getClassById( e->class_id );
        if( lib_class && lib_class->cls() && lib_class->cls()->table.obj_new )
        {
            if( lib_class->cls()->size == sizeof(lh_class) &&
                lib_class->table()->size == sizeof(lh_instance_calltable) &&
                lib_class->table()->o.size == sizeof(lh_object_calltable) )
            {
                const void *obj = lib_class->obj_new();
                if( obj )
                {
                    lib_inst = new LibInstance( e->inst_id, lib_class->cls(), obj, this );
                }
                else
                    qWarning() << "LibPlugin::createInstance() fails: obj_new() returned NULL for" << e->class_id;
            }
            else
                qWarning() << "LibPlugin::createInstance() won't instantiate, wrong checksize(s) for" << e->class_id;
        }
        else
            qWarning() << "LibPlugin::createInstance() can't instantiate " << e->class_id;
        return true;
    }

    if( event->type() == EventDestroyInstance::type() )
    {
        EventDestroyInstance *e = static_cast<EventDestroyInstance*>(event);
        foreach( QObject *o, children() )
        {
            Q_ASSERT( qobject_cast<LibInstance *>(o) );
            LibInstance *lib_inst = static_cast<LibInstance *>(o);
            if( lib_inst && lib_inst->id() == e->id )
            {
                delete lib_inst;
                return true;
            }
        }
        return true;
    }

    return LibLibrary::event( event );
}

#endif
