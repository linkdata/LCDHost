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
#include <QApplication>
#include <QSettings>
#include <QBuffer>
#include <QMutex>
#include <QMutexLocker>
#include <QMetaMethod>
#include <QUrl>

#include "LCDHost.h"
#include "MainWindow.h"
#include "EventCreateInstance.h"
#include "EventClassLoaded.h"
#include "EventClassRemoved.h"
#include "EventLibraryLoaded.h"
#include "EventLayoutChanged.h"
#include "AppState.h"
#include "AppPlugin.h"
#include "AppInstance.h"
#include "AppClass.h"

AppPlugin::AppPlugin( QFileInfo fi ) : AppLibrary( fi )
{
    type_ = Plugin;
    return;
}

AppPlugin::~AppPlugin()
{
    disconnect();
    stop();
    return;
}

void AppPlugin::bindClass( AppClass *app_class )
{
    QList<AppInstance*> list;

    for( QList<QObject*>::const_iterator i = AppLibrary::orphanage()->children().constBegin();
         i != AppLibrary::orphanage()->children().constEnd(); ++i )
    {
        Q_ASSERT( qobject_cast<AppInstance *>(*i) );
        list.append( static_cast<AppInstance *>(*i) );
    }

    mutex_.lock();
    for( QList<QObject*>::const_iterator i = children().constBegin(); i != children().constEnd(); ++i )
    {
        Q_ASSERT( qobject_cast<AppInstance *>(*i) );
        list.append( static_cast<AppInstance *>(*i) );
    }
    mutex_.unlock();

    foreach( AppInstance *app_inst, list )
    {
        if( app_inst->classId() == app_class->id() )
        {
            app_inst->setParent( this );
            app_inst->setClassSize( app_class->size() );
            postLibEvent( id(), new EventCreateInstance( app_class->id(), app_inst->id() ) );
        }
    }

    return;
}

bool AppPlugin::event( QEvent *event )
{
    if( event->type() == EventLibraryLoaded::type() )
    {
        bool retv = AppLibrary::event( event );
        return retv;
    }

    if( event->type() == EventClassLoaded::type() )
    {
        // create a corresponding AppClass
        EventClassLoaded *e = static_cast<EventClassLoaded*>(event);
        AppClass *app_class = new AppClass( this, e->id, e->path, e->name, e->size );
        if( app_class )
        {
            bindClass( app_class );
            if( mainWindow ) mainWindow->classTree()->insertClass( app_class );
        }
        return true;
    }

    if( event->type() == EventClassRemoved::type() )
    {
        EventClassRemoved *e = static_cast<EventClassRemoved*>(event);
        AppClass *app_class = AppClass::fromId( e->id );
        if( app_class )
        {
            if( mainWindow ) mainWindow->classTree()->removeClass( app_class );
            delete app_class;
        }
        return true;
    }

    return AppLibrary::event(event);
}

void AppPlugin::layoutChanged( QString dir, QString name )
{
    postLibEvent( id(), new EventLayoutChanged(dir,name) );
}
#endif
