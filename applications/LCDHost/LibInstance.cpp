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
#include <QImage>
#include <QString>

// #include "LCDHost.h"
#include "AppState.h"
#include "AppInstance.h"
#include "LibInstance.h"

#include "EventInstanceRendered.h"
#include "EventInstanceRender.h"
#include "EventRequestRender.h"

LibInstance::LibInstance(
    AppId id,
    const LibClass& cls,
    QString name,
    const void *obj,
    LibLibrary *parent )
    : LibObject( id, name, obj, parent ), class_(cls)
{
    Q_ASSERT( obj );
    return;
}

LibInstance::~LibInstance()
{
    return;
}

QString LibInstance::init()
{
    return LibObject::init();
}

void LibInstance::term()
{
    if(!id().isValid()) {
      qDebug() << "LibInstance::term()" << metaObject()->className() << objectName() << "has no id";
      return;
    }
    LibObject::term();
    obj_delete();
    class_.clear();
}

void LibInstance::requestRender()
{
    id().postAppEvent( new EventRequestRender(id()) );
}

void LibInstance::eventRender( int w, int w_method, int w_mod, int h, int h_method, int h_mod )
{
    QImage image;
    QImage *qimage_retv;
    QSize plugin_size;
    const lh_blob *blob;
    bool w_missing, h_missing;

    if (AppState* as = AppState::instance())
      as->layout_rps_add();

    if( w_method == LH_METHOD_ABSOLUTE ) w = w_mod;
    if( h_method == LH_METHOD_ABSOLUTE ) h = h_mod;

    w_missing = h_missing = false;

    // prerender
    obj_prerender();

    // if we want plugin size suggestion, ask it
    if( w < 1 )
    {
        w = obj_width(h);
        if( w < 0 )
        {
            w = width();
            w_missing = true;
        }
        if( w >= 0 )
        {
            plugin_size.setWidth( w );
            w = AppInstance::valueFromMod( w_method, w_mod, 0, w, 0, w );
        }
    }

    if( h < 1 )
    {
        h = obj_height(w);
        if( h < 0 )
        {
            h = height();
            h_missing = true;
        }
        if( h >= 0 )
        {
            plugin_size.setHeight( h );
            h = AppInstance::valueFromMod( h_method, h_mod, 0, h, 0, h );
        }
    }

    Q_ASSERT( w >= 0 );
    Q_ASSERT( h >= 0 );

    // First try rendering to qimage
    qimage_retv = static_cast<QImage*>( obj_render_qimage(w,h) );
    if( qimage_retv ) image = qimage_retv->copy();

    // Second, try rendering to blob
    if( image.isNull() )
    {
        blob = obj_render_blob(w,h);
        if( blob != NULL && blob->sign == 0xDEADBEEF )
        {
            QByteArray ary((const char *) blob->data, (int) blob->len );
            if( !image.loadFromData( ary ) )
            {
                qDebug()
                        << libLibrary()->objectName()
                        << "image loader failed for"
                        << blob->len << "bytes.";
            }
        }
    }

    if( !image.isNull() )
    {
        // handle the case of letting the plugin set the size,
        // but the plugin didn't know the size before rendering
        if( w_missing )
        {
            w = image.width();
            plugin_size.setWidth( w );
            w = AppInstance::valueFromMod( w_method, w_mod, 0, w, 0, w );
        }
        if( h_missing )
        {
            h = image.height();
            plugin_size.setHeight( h );
            h = AppInstance::valueFromMod( h_method, h_mod, 0, h, 0, h );
        }

        // scale to application specified if plugin didn't do it
        if( image.width() != w || image.height() != h )
            image = image.scaled( w, h, Qt::IgnoreAspectRatio, Qt::FastTransformation );
    }

    id().postAppEvent( new EventInstanceRendered(id(),image,plugin_size) );
    return;
}

bool LibInstance::event( QEvent *event )
{
    if( LibObject::event(event) ) return true;
    Q_ASSERT( event->type() >= QEvent::User );
    Q_ASSERT( event->type() <= QEvent::MaxUser );
    Q_ASSERT( id().isValid() );

    if( event->type() == EventInstanceRender::type() )
    {
        Q_ASSERT( id().isValid() );
        EventInstanceRender *e = static_cast<EventInstanceRender*>(event);
        if( isSetupCompleted() )
            eventRender(e->width,e->w_method,e->w_mod,e->height,e->h_method,e->h_mod);
        return true;
    }

    qWarning() << "LibInstance::event() unhandled user event" << EventBase::name(event->type()) << "for" << objectName();
    return false;
}
