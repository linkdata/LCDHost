/**
  \file     LH_QtDevice.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

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
#include "LH_QtDevice.h"

/**
  Device stubs.
  */
#define RECAST(o) reinterpret_cast<LH_QtDevice*>(o)

static const char* obj_open(void*obj) { return RECAST(obj)->open(); }
static const char* obj_render_qimage(void*obj,void*qi) { return RECAST(obj)->render_qimage(static_cast<QImage*>(qi)); }
static const char* obj_render_argb32(void*obj,int w,int h,const void*p) { return RECAST(obj)->render_argb32(w,h,p); }
static const char* obj_render_mono(void*obj,int w,int h,const void*p) { return RECAST(obj)->render_argb32(w,h,p); }
static const char* obj_get_backlight(void*obj,lh_device_backlight*p) { return RECAST(obj)->get_backlight(static_cast<lh_device_backlight*>(p)); }
static const char* obj_set_backlight(void*obj,lh_device_backlight*p) { return RECAST(obj)->set_backlight(static_cast<lh_device_backlight*>(p)); }
static const char* obj_close(void*obj) { return RECAST(obj)->close(); }


LH_QtDevice::LH_QtDevice( LH_QtObject *parent ) : LH_QtObject( parent )
{
    memset( &lh_dev_, 0, sizeof(lh_dev_) );
    LH_QtObject::build_object_calltable( & lh_dev_.objtable );
    lh_dev_.size = sizeof(lh_dev_);
    lh_dev_.obj = this;
    lh_dev_.table.size = sizeof(lh_dev_.table);
    lh_dev_.table.obj_open = obj_open;
    lh_dev_.table.obj_render_qimage = obj_render_qimage;
    lh_dev_.table.obj_render_argb32 = obj_render_argb32;
    lh_dev_.table.obj_render_mono = obj_render_mono;
    lh_dev_.table.obj_get_backlight = obj_get_backlight;
    lh_dev_.table.obj_set_backlight = obj_set_backlight;
    lh_dev_.table.obj_close = obj_close;
    return;
}

LH_QtDevice::~LH_QtDevice()
{
    memset(&lh_dev_, 0, sizeof(lh_dev_));
    return;
}

void LH_QtDevice::userTerm()
{
    leave();
    return;
}

void LH_QtDevice::arrive()
{
    Q_ASSERT( !devid().isEmpty() );
    Q_ASSERT( !size().isEmpty() );
    Q_ASSERT( depth() > 0 );
    if(depth() > 0)
    {
        if(parent() && parent()->callable())
            parent()->callback(lh_cb_arrive,lh_dev());
        else
            qCritical() << "LH_QtDevice::arrive():"
                        << metaObject()->className() << objectName()
                        << "no callback in parent"
                        << (parent() ? parent()->metaObject()->className() : "NULL")
                           ;
    }
    return;
}

void LH_QtDevice::leave()
{
    if(depth() > 0)
    {
        if(parent() && parent()->callable())
            parent()->callback(lh_cb_leave,lh_dev());
        else
            qCritical() << "LH_QtDevice::leave():"
                        << metaObject()->className() << objectName()
                        << "no callback in parent"
                        << (parent() ? parent()->metaObject()->className() : "NULL")
                           ;
    }
}

void LH_QtDevice::setDevid(const QString &a)
{
    devid_ = a;
    devid_latin1_ = devid_.toLatin1();
    lh_dev_.devid = devid_latin1_.constData();
    return;
}

void LH_QtDevice::setObjectName(const QString &s)
{
    QObject::setObjectName(s);
    name_utf8_ = s.toUtf8();
    lh_dev_.name = name_utf8_.constData();
    return;
}
