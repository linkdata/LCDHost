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


#include <QtDebug>
#include <QMimeData>
#include <QDrag>
#include <QLabel>
#include <QPainter>
#include <QApplication>
#include <QUrl>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppInstancePreview.h"


AppInstancePreview::AppInstancePreview(QWidget * parent, Qt::WindowFlags f ) : QFrame( parent, f)
{
    app_inst = NULL;
    dragStartPosition = QPoint();
    setMouseTracking(true);
}

AppInstancePreview::~AppInstancePreview()
{
    clear();
}

void AppInstancePreview::clear()
{
    if( app_inst )
    {
        app_inst->term();
        delete app_inst;
        app_inst = NULL;
    }
    return;
}

void AppInstancePreview::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && pixrect.contains( event->pos() ) )
        dragStartPosition = event->pos();
}

void AppInstancePreview::mouseMoveEvent(QMouseEvent *event)
{
    if( pixrect.contains( event->pos() ) ) setCursor( Qt::OpenHandCursor );
    else unsetCursor();

    if( app_inst == NULL ) return;
    if( dragStartPosition.isNull() ) return;
    if( !(event->buttons() & Qt::LeftButton) ) return;
    if( (event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance() ) return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    QList<QUrl> list;
    list << app_inst->url();
    mimeData->setUrls( list );
    drag->setMimeData( mimeData );
    drag->setPixmap( pixmap );
    // drag->setHotSpot( event->pos() - pixrect.topLeft() );
    drag->setHotSpot( QPoint(0,0) );
    drag->exec( Qt::CopyAction );
}

void AppInstancePreview::clearAppInst()
{
    pixmap = QPixmap();
    app_inst = NULL;
}

void AppInstancePreview::paintEvent( QPaintEvent * )
{
    QPainter painter( this );
    pixrect.setSize( pixmap.size() );
    pixrect.moveCenter( rect().center() );
    painter.drawPixmap( pixrect, pixmap );
    return;
}

void AppInstancePreview::recalcPixmap()
{
    QPixmap pm;
    QSize newSize;
    QSize copySize;

    if( mainWindow == 0 || app_inst == 0 )
    {
        if( !pixmap.isNull() )
        {
            pixmap = QPixmap();
            updateGeometry();
            update();
        }
        return;
    }

    switch( mainWindow->renderMethod() )
    {
    case 0:
        pm = QPixmap::fromImage( app_inst->image() );
        break;
    case 1:
        pm = app_inst->pixmap();
        break;
    }

    newSize = pm.size();
    newSize.rwidth() *= mainWindow->currentLayoutZoom;
    newSize.rheight() *= mainWindow->currentLayoutZoom;
    copySize = newSize;
    copySize.scale( width(), newSize.height(), Qt::KeepAspectRatio );
    copySize.rwidth() ++;
    copySize.rheight() ++;

    QPixmap copy( copySize );
    copy.fill( Qt::transparent );
    QPainter painter( &copy );
    painter.drawPixmap( 0, 0, pm.scaled( newSize ) );
    QRectF adj = copy.rect().adjusted(0,0,-1,-1);
    painter.setPen( QColor( 200, 200, 200, 128 ) );
    painter.drawRect( adj );

    if( pm.isNull() )
    {
        painter.drawLine( adj.topLeft(), adj.bottomRight() );
        painter.drawLine( adj.bottomLeft(), adj.topRight() );
    }

    if( pixmap.size() != copy.size() )
    {
        setMinimumSize( copy.size() );
        updateGeometry();
    }

    pixmap = copy;
    update();
}

void AppInstancePreview::previewClass( QString classId )
{
    AppClass *app_class =  AppLibrary::getClass(classId);
    if( app_class )
    {
        clear();
        app_inst = new AppInstance( app_class, "Preview" );
        app_inst->setParent( app_class->parent() );
        app_inst->init();
        app_inst->setupComplete();
        connect( app_inst, SIGNAL(rendered()), this, SLOT(recalcPixmap()) );
        connect( app_inst, SIGNAL(destroyed()), this, SLOT(clearAppInst()) );
    }
    else
        qDebug() << "AppInstancePreview::previewClass(): "<<classId<<"not found";
    return;
}
