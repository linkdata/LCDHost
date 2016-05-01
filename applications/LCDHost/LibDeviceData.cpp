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

#include "LibDeviceData.h"

LibDeviceData *LibDeviceData::first_ = 0;
QMutex LibDeviceData::mutex_;

bool LibDeviceData::exists( QString id )
{
    bool retv = false;
    LibDeviceData *dd;
    if( mutex_.tryLock() )
    {
        for( dd = first_; dd; dd = dd->next_ )
        {
            if( dd->devId() == id )
            {
                retv = true;
                break;
            }
        }
        mutex_.unlock();
    }
    return retv;
}

LibDeviceData *LibDeviceData::add( const void *obj, lh_device *dev )
{
    LibDeviceData *dd = 0;
    if( obj && dev && dev->devid && mutex_.tryLock() )
    {
        QString id = QString::fromUtf8( dev->devid );
        for( dd = first_; dd; dd = dd->next_ )
        {
            if( dd->devId() == id )
            {
                mutex_.unlock();
                return dd;
            }
        }
        dd = new LibDeviceData(obj,dev);
        dd->next_ = first_;
        first_ = dd;
    }
    return dd;
}

void LibDeviceData::remove( LibDeviceData *to_remove )
{
    LibDeviceData *last = 0;
    for( LibDeviceData *dd = first_; dd; dd = dd->next_ )
    {
        if( dd == to_remove )
        {
            if( last ) last->next_ = dd->next_;
            else first_ = dd->next_;
            dd->next_ = 0;
            delete dd;
            break;
        }
        last = dd;
    }
    return;
}

LibDeviceData* LibDeviceData::lock( QString id )
{
    LibDeviceData *retv = 0;
    if( mutex_.tryLock() )
    {
        for( retv = first_; retv; retv = retv->next_ )
        {
            if( retv->devId() == id )
                return retv;
        }
        mutex_.unlock();
    }
    return retv;
}

LibDeviceData* LibDeviceData::lockObj( const void *obj )
{
    LibDeviceData *retv = 0;
    if( mutex_.tryLock() )
    {
        for( retv = first_; retv; retv = retv->next_ )
        {
            if( retv->obj() == obj )
                return retv;
        }
        mutex_.unlock();
    }
    return retv;
}

void LibDeviceData::unlock()
{
    mutex_.unlock();
}

LibDeviceData::LibDeviceData( const void *obj, const lh_device *dev )
{
    Q_ASSERT( dev );
    Q_ASSERT( dev->size == sizeof(lh_device) );
    id_.clear();
    obj_ = obj;
    depth_ = 0;
    autoselect_ = false;
    memset( &objtable_, 0, sizeof(objtable_) );
    memset( &table_, 0, sizeof(table_) );

    if( dev && dev->size == sizeof(lh_device) )
    {
        if( dev->id ) devid_ = QString::fromUtf8( dev->id );
        if( dev->name ) name_ = QString::fromUtf8( dev->name );
        size_ = QSize( dev->width, dev->height );
        depth_ = dev->depth;
        autoselect_ = !dev->noauto;
        if( dev->table.size == sizeof(table_) ) memcpy( &table_, &dev->table, sizeof(table_) );
        if( dev->objtable.size == sizeof(objtable_) ) memcpy( &objtable_, dev->objtable, sizeof(objtable_) );
        if( dev->logo && dev->logo->sign == 0xDEADBEEF )
            logo_ = QImage::fromData( QByteArray( (const char *) dev->logo->data, (int) dev->logo->len ) );
        for( int i=0; i<LH_DEVICE_MAXBUTTONS; i++ )
            if( dev->button[i] ) buttonname_[i] = QString::fromUtf8(dev->button[i]);
    }

    return;
}
