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


#include "AppId.h"
#include "AppState.h"
#include "AppLibrary.h"
#include "AppObject.h"
#include "EventNotify.h"
#include "EventCallback.h"

#include <QCoreApplication>

#define WAITTIME 10000

QAtomicInt AppId::masterid_(1);
QMutex AppId::mutex_;
QVector<AppObject*> AppId::app_objects_;
QVector<LibObject*> AppId::lib_objects_;

QList<AppObject*> AppId::appObjects()
{
    QList<AppObject*> retv;
    if( mutex_.tryLock(WAITTIME) )
    {
        foreach( AppObject *app_obj, app_objects_ )
            if( app_obj ) retv.append( app_obj );
        mutex_.unlock();
    }
    return retv;
}

QList<LibObject*> AppId::libObjects()
{
    QList<LibObject*> retv;
    if( mutex_.tryLock(WAITTIME) )
    {
        foreach( LibObject *lib_obj, lib_objects_ )
            if( lib_obj ) retv.append( lib_obj );
        mutex_.unlock();
    }
    return retv;
}

int AppId::countAppObjects()
{
    int retv = 0;
    if( mutex_.tryLock(WAITTIME) )
    {
        foreach( AppObject *app_obj, app_objects_ )
            if( app_obj ) retv ++;
        mutex_.unlock();
    }
    return retv;
}

int AppId::countLibObjects()
{
    int retv = 0;
    if( mutex_.tryLock(WAITTIME) )
    {
        foreach( LibObject *lib_obj, lib_objects_ )
            if( lib_obj ) retv ++;
        mutex_.unlock();
    }
    return retv;
}

void AppId::notify( int n, void *p )
{
    if( mutex_.tryLock(WAITTIME) )
    {
        QVector<LibObject*>::const_iterator it = lib_objects_.constBegin();
        for( it = lib_objects_.constBegin(); it != lib_objects_.constEnd(); ++ it )
        {
            LibObject * lib_obj = *it;
            if( lib_obj ) QCoreApplication::postEvent( lib_obj, new EventNotify(n,p) );
        }
        mutex_.unlock();
    }
    return;
}

bool AppId::hasAppObject() const
{
    bool retv = false;
    if( mutex_.tryLock(WAITTIME) )
    {
        if( id_ < app_objects_.size() )
            retv = (app_objects_.at(id_) != NULL);
        mutex_.unlock();
    }
    else
    {
        qWarning() << "AppId::hasAppObject(): deadlock detected";
    }
    return retv;
}

AppObject *AppId::appObject() const
{
    AppObject* retv = 0;
    if( mutex_.tryLock(WAITTIME) )
    {
        if( id_ < app_objects_.size() )
            retv = app_objects_.at(id_);
        mutex_.unlock();
    }
    return retv;
}

LibObject *AppId::lock() const
{
    LibObject *retv = 0;
    if( mutex_.tryLock(WAITTIME) )
    {
        if( id_ < lib_objects_.size() )
            retv = lib_objects_.at(id_);
        if( !retv ) mutex_.unlock();
    }
    return retv;
}

void AppId::unlock() const
{
    mutex_.unlock();
}

bool AppId::hasLibObject() const
{
    bool retv = false;
    if( mutex_.tryLock(WAITTIME) )
    {
        if( id_ < lib_objects_.size() )
            retv = (lib_objects_.at(id_) != NULL);
        mutex_.unlock();
    }
    else
    {
        qWarning() << "AppId::hasLibObject(): deadlock detected";
    }
    return retv;
}

void AppId::setAppObject( AppObject *obj )
{
    if( mutex_.tryLock(WAITTIME) )
    {
        if( app_objects_.size() <= id_ )
            app_objects_.resize(id_+1);
        app_objects_[id_] = obj;
        mutex_.unlock();
        Q_ASSERT( (obj!=NULL) == hasAppObject() );
    }
    else
    {
        qWarning() << "AppId::setAppObject(): deadlock detected";
    }
    return;
}

void AppId::setLibObject( LibObject *obj )
{
    if( mutex_.tryLock(WAITTIME) )
    {
        if( lib_objects_.size() <= id_ )
            lib_objects_.resize(id_+1);
        lib_objects_[id_] = obj;
        mutex_.unlock();
    }
    else
    {
        qWarning() << "AppId::setLibObject(): deadlock detected";
    }
    return;
}

void AppId::postAppEvent( QEvent *event, int priority )
{
    Q_ASSERT( event != NULL );
    if( mutex_.tryLock(WAITTIME) )
    {
        if( id_ < app_objects_.size() )
        {
            AppObject *app_object = app_objects_.at(id_);
            if( app_object )
            {
                QCoreApplication::postEvent( app_object, event, priority );
                mutex_.unlock();
                return;
            }
        }
        mutex_.unlock();
#ifndef QT_NO_DEBUG
        qDebug() << "AppId::postAppEvent(): no recipient" << toString() <<
                    "for" << EventBase::name(event->type())
                    ;
#endif
    }
    else
    {
        qWarning() << "AppId::postAppEvent(): deadlock detected sending" <<
                      EventBase::name(event->type()) << "to" << toString();
    }
    delete event;
    return;
}

void AppId::postLibEvent( QEvent *event, int priority )
{
    Q_ASSERT( event != NULL );
    if( mutex_.tryLock(WAITTIME) )
    {
        if( id_ < lib_objects_.size() )
        {
            LibObject *lib_object = lib_objects_.at(id_);
            if( lib_object )
            {
                QCoreApplication::postEvent( lib_object, event, priority );
                mutex_.unlock();
                return;
            }
        }
        mutex_.unlock();
    }
    else
    {
        qWarning() << "AppId::postLibEvent(): deadlock detected sending" <<
                      EventBase::name(event->type()) << "to" << toString();
    }
    delete event;
    return;
}

QString AppId::toString() const
{
    QString retv = QString::number(toInt());
    retv.append(':');
    if( mutex_.tryLock(WAITTIME) )
    {
        if( id_ < app_objects_.size() )
        {
            if( AppObject *app_obj = app_objects_.value(id_) )
            {
                if( app_obj->objectName().isEmpty() )
                    retv.append("(blank)");
                else
                    retv.append( app_obj->objectName() );
            }
            else if( LibObject *lib_obj = lib_objects_.value(id_) )
            {
                if( lib_obj->objectName().isEmpty() )
                    retv.append("(blank)");
                else
                    retv.append( lib_obj->objectName() );
            }
            else
                retv.append("(null)");
        }
        else
        {
            retv.append("(bounds)");
        }
        mutex_.unlock();
    }
    else
    {
        retv.append("(nolock)");
    }
    return retv;
}


AppId AppId::fromString(QString s)
{
    if( !s.isEmpty() )
    {
        int len = s.indexOf(':');
        if( len > 0 )
        {
            int val = s.left( len ).toInt();
            return AppId( val );
        }
    }
    return AppId(0);
}

QDebug operator<<(QDebug dbg, AppId id)
{
    dbg << id.toString();
    return dbg;
}

