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

#include <QtGlobal>
#include <QDebug>
#include <QWidget>
#include <QMutex>
#include <QMutexLocker>
#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QApplication>
#include <QTimer>
#include <QIcon>
#include <QThread>

#include "LCDHost.h"
#include "AppState.h"
#include "AppObject.h"
#include "AppLibrary.h"
#include "AppRawInput.h"
#include "RawInputDevice.h"
#include "RawInputEngine.h"
#include "EventRawInput.h"
#include "EventRawInputRefresh.h"

#if defined(Q_OS_MAC) && !defined(Q_WS_QWS)
#include "RawInputEngine_mac.h"
#define RAWINPUT_ENGINE RawInputEngine_mac
#endif // Q_OS_MAC

#if defined(Q_OS_WIN)
#include "RawInputEngine_win.h"
#define RAWINPUT_ENGINE RawInputEngine_win
#endif // Q_OS_WIN

#if defined(Q_OS_LINUX)
#include "RawInputEngine_x11.h"
#define RAWINPUT_ENGINE RawInputEngine_x11
#endif // Q_OS_LINUX

AppRawInput* AppRawInput::instance_ = NULL;

void AppRawInput::CreateInstance() {
  Q_ASSERT(!instance_);
  if (!instance_)
    instance_ = new AppRawInput();
  return;
}

void AppRawInput::DestroyInstance() {
  Q_ASSERT(instance_);
  if (AppRawInput* ri = instance_) {
    instance_ = NULL;
    delete ri;
  }
  return;
}

AppRawInput::AppRawInput()
  : QAbstractListModel()
  , isrefreshing_(false)
  , listeners_(0)
  , e(NULL)
{
    e = new RAWINPUT_ENGINE( this );
}

AppRawInput::~AppRawInput()
{
    delete e;
}

void AppRawInput::clear()
{
    e->clear();
}

// List interface
int AppRawInput::rowCount( const QModelIndex &parent ) const
{
    if( parent.isValid() ) return 0;
    return e->list().size();
}

QVariant AppRawInput::data( const QModelIndex & index, int role ) const
{
    if( !index.isValid() || index.column() ) return QVariant();
    if( index.row() < 0 || index.row() >= e->list().size() ) return QVariant();
    QMutexLocker locker( e->listMutex() );
    if( role == Qt::CheckStateRole )
    {
        if( e->list().at( index.row() )->isCaptured() ) return true;
        else return false;
    }
    if( role == Qt::DisplayRole )
    {
        return e->list().at( index.row() )->name();
    }
    return QVariant();
}

bool AppRawInput::setData( const QModelIndex & index, const QVariant & value, int role )
{
    Q_UNUSED(value);

    if( !index.isValid() || index.column() ) return false;
    if( index.row() < 0 || index.row() >= e->list().size() ) return false;
    if( role == Qt::CheckStateRole )
    {
        if( e->list().at( index.row() )->setCapture( !e->list().at( index.row() )->isCaptured() ) )
        {
            emit dataChanged( index, index );
            return true;
        }
        return false;
    }
    return false;
}

int AppRawInput::columnCount( const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

Qt::ItemFlags AppRawInput::flags( const QModelIndex & index ) const
{
    if( index.isValid() ) return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
    return 0;
}

QVariant AppRawInput::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( section != 0 ) return QVariant();
    if( role == Qt::DisplayRole && orientation == Qt::Horizontal ) return "Name";
    return QVariant();
}

bool AppRawInput::event( QEvent *event )
{
    if( event->type() == EventRawInput::type() )
    {
        EventRawInput *e = static_cast<EventRawInput*>(event);
        broadcast( e );
        return true;
    }

    if( event->type() == EventRawInputRefresh::type() )
    {
        if( !isrefreshing_ )
        {
            // Delay 500 ms before refresh
            isrefreshing_ = true;
            QTimer::singleShot(500,this,SLOT(refresh()));
        }
        return true;
    }
    return QAbstractListModel::event( event );
}

void AppRawInput::refresh()
{
    isrefreshing_ = false;
    if( e )
    {
        QStringList selected;
        beginResetModel();
        selected = getCapturedList();
        e->clear();
        e->scan();
        setCapturedList( selected );
        endResetModel();
    }
}

RawInputDevice AppRawInput::device(int n)
{
    if( n>=0 && n<e->list().size() ) return RawInputDevice( *e->list().at(n) );
    return RawInputDevice(e);
}

RawInputDevice AppRawInput::device(QByteArray devid)
{
    QMutexLocker locker( e->listMutex() );
    for( int i=0; i<e->list().size(); i++ )
    {
        if( e->list().at(i)->devid() == devid ) return RawInputDevice( *e->list().at(i) );
    }
    return RawInputDevice(e,devid,devid);
}

QString AppRawInput::itemName( QByteArray devid, QString control, int item, int val, int flags )
{
    QMutexLocker locker( e->listMutex() );
    QList<RawInputDevice*>::const_iterator i;
    for( i = e->list().constBegin(); i != e->list().constEnd(); i ++ )
    {
        if( (*i)->devid() == devid ) return (*i)->itemName(item,val,flags);
    }
    return control;
}

QStringList AppRawInput::getCapturedList()
{
    QStringList list;
    QMutexLocker locker( e->listMutex() );
    foreach( RawInputDevice *rid, e->list() )
    {
        if( rid->isCaptured() )
            list.append( rid->devid() );
    }
    return list;
}

void AppRawInput::setCapturedList( QStringList list )
{
    foreach( RawInputDevice *rid, e->list() )
        rid->setCapture( list.contains( rid->devid() ) );
    return;
}

QString AppRawInput::describeEvent( QByteArray devid, QString control, int item, int val, int flags )
{
    if( !flags && !item ) return QString();
    if( flags & EventRawInput::Button )
    {
        return tr("%1 %2 @ %3")
                .arg( itemName(devid,control,item,val,flags) )
                .arg( (flags&EventRawInput::Down) ? tr("pressed") : tr("released") )
                .arg( device( devid ).name() )
                ;
    }
    Q_ASSERT( flags & EventRawInput::Value );
    return tr("%1 %2 @ %3")
            .arg( itemName(devid,control,item,val,flags) )
            .arg( (flags&EventRawInput::Neg) ? tr("negative") : tr("positive") )
            .arg( device( devid ).name() )
            ;
}

QString AppRawInput::describeControl( QByteArray devid, QString control, int item, int val, int flags )
{
    if( !flags && !item ) return QString();
    if( flags & EventRawInput::Button )
    {
        return tr("%1 @ %2")
                .arg( itemName(devid,control,item,val,flags) )
                .arg( device( devid ).name() )
                ;
    }
    Q_ASSERT( flags & EventRawInput::Value );
    return tr("%1 @ %2")
            .arg( itemName(devid,control,item,val,flags) )
            .arg( device( devid ).name() )
            ;
}

void AppRawInput::broadcast( EventRawInput *evt )
{
    Q_ASSERT( QThread::currentThread() == this->thread() );

    emit rawInput( evt->devid, evt->control, evt->item, evt->value, evt->flags );

    foreach( AppObject *app_obj, AppId::appObjects() )
    {
        if( app_obj ) app_obj->input( evt ); // app_obj->input( dev_id, item, val, flags );
    }
}
