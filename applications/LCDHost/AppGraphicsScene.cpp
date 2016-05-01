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
#include <QUrl>
#include <QPainter>
// #include <QKeyEvent>
#include <QGraphicsView>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppInstance.h"
#include "AppInstanceTree.h"
#include "AppGraphicsScene.h"

AppGraphicsScene::AppGraphicsScene(MainWindow* parent)
    : QGraphicsScene(parent)
{
    monochrome_ = false;
    markoutline_ = true;
    setBackgroundBrush( Qt::white );
    setItemIndexMethod( QGraphicsScene::NoIndex );
}

MainWindow* AppGraphicsScene::mainWindow() const
{
    return static_cast<MainWindow*>(parent());
}

Qt::DropActions AppGraphicsScene::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList AppGraphicsScene::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

void AppGraphicsScene::dragMoveEvent( QGraphicsSceneDragDropEvent * event )
{
    const QMimeData *data;
    data = event->mimeData();
    if( data == NULL || event->proposedAction() == Qt::IgnoreAction ) return;

    if( data->hasUrls() )
    {
        QPointF diff;
        AppInstance *app_inst;
        AppInstance *first_item;

        first_item = NULL;
        diff = QPointF(0,0);
        foreach( QUrl url, data->urls() )
        {
            if( url.scheme() == "lcdhostobject" )
            {
                app_inst = qobject_cast<AppInstance*>(AppId::fromString( url.path()).appObject());
                if( app_inst && app_inst->isModifiable() )
                {
                    layoutModified();
                    if( first_item == NULL ) first_item = app_inst;
                    if( first_item && first_item != app_inst ) diff = app_inst->scenePos() - first_item->scenePos();
                    app_inst->setScenePos( event->scenePos() + diff );
                    if(app_inst->isSelected())
                        mainWindow()->refreshInstanceDetails();
                }
            }
        }
    }
}

void AppGraphicsScene::dropEvent( QGraphicsSceneDragDropEvent * event )
{
    const QMimeData *data;
    data = event->mimeData();
    if( data == NULL || event->proposedAction() == Qt::IgnoreAction ) return;

    if( data->hasUrls() )
    {
        AppInstance *app_inst;

        foreach( QUrl url, data->urls() )
        {
            if( url.scheme() == "lcdhostobject" )
            {
                app_inst = qobject_cast<AppInstance*>(AppId::fromString( url.path() ).appObject());
                if( app_inst && app_inst->parentItem() == NULL)
                {
                    AppInstance *app_copy = new AppInstance(*app_inst);
                    app_copy->init();
                    app_copy->setupComplete();
                    if( mainWindow()->tree()->insertAppInstance( app_copy, QModelIndex(), event->scenePos() ) )
                    {
                        layoutModified();
                        clearSelection();
                        app_copy->setSelected( true );
                        this->views().first()->setFocus();
                    }
                    else
                    {
                        app_copy->term();
                        delete app_copy;
                    }
                }
            }

            if( url.scheme() == "lcdhostclass" )
            {
                AppClass *app_class = AppLibrary::getClass( url.path() );
                if( app_class )
                {
                    AppInstance *app_inst = new AppInstance( app_class );
                    app_inst->init();
                    app_inst->setupComplete();
                    if( mainWindow()->tree()->insertAppInstance( app_inst, QModelIndex(), event->scenePos() ) )
                    {
                        layoutModified();
                        clearSelection();
                        app_inst->setSelected( true );
                        this->views().first()->setFocus();
                        // qDebug() << "AppGraphicsScene::dropEvent()" << app_inst->objectName() << app_inst->pos();
                    }
                    else
                    {
                        app_inst->term();
                        delete app_inst;
                    }
                }
            }
        }
    }

    return;
}

void AppGraphicsScene::drawBackground( QPainter * painter, const QRectF & rectf )
{
    QRectF srectf =
            sceneRect();
    QRect rect( (int)(rectf.x()-2), (int)(rectf.y()-2), (int)(rectf.width()+4), (int)(rectf.height()+4) );
    QRect scenerect( (int)(srectf.x()), (int)(srectf.y()), (int)(srectf.width()), (int)(srectf.height()) );
    QRegion region( rect );
    QRegion innerregion( scenerect.intersected(rect) );
    QRegion outerregion = region.subtracted( innerregion );

    foreach( QRect orect, outerregion.rects() )
    {
        painter->fillRect( orect, Qt::lightGray );
    }
    foreach( QRect irect, innerregion.rects() )
    {
        painter->fillRect( irect, Qt::white );
    }
    return;
}

void AppGraphicsScene::keyPressEvent( QKeyEvent * event )
{
    foreach( QGraphicsItem *gitem, selectedItems() )
    {
        AppInstance *pitem = static_cast<AppInstance*>(gitem);
        pitem->keyPressEvent( event );
    }
    return;
}

void AppGraphicsScene::render( QPainter * painter, const QRectF & target, const QRectF & source, Qt::AspectRatioMode aspectRatioMode)
{
    markoutline_ = false;
    QGraphicsScene::render( painter, target, source, aspectRatioMode );
    markoutline_ = true;
    return;
}

