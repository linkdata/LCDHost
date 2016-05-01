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


#include <QMouseEvent>
#include <QMimeData>
#include <QModelIndexList>
#include <QPainter>
#include <QDrag>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppClass.h"
// #include "AppClassList.h"
#include "AppClassTree.h"
#include "AppInstance.h"
#include "AppInstanceTree.h"
#include "AppTreeView.h"

void AppTreeView::selectionChanged( const QItemSelection & selected, const QItemSelection & deselected )
{
    QTreeView::selectionChanged( selected, deselected );
    emit selectionChange( selected, deselected );
    return;
}

void AppTreeView::currentChanged( const QModelIndex & current, const QModelIndex & previous )
{
    AppClass *app_class;
    AppClassTree *app_class_tree = qobject_cast<AppClassTree *>(model());

    QTreeView::currentChanged( current, previous );
    if( app_class_tree )
    {
        QStringList id_list;
        id_list = app_class_tree->itemFromIndex( current )->data().toStringList();
        if( !id_list.isEmpty() )
        {
            app_class =  AppLibrary::getClass( id_list.first() );
            if( app_class ) emit wantPreview( app_class->id() );
        }
    }
    return;
}

void AppTreeView::mousePressEvent(QMouseEvent *event)
{
    QTreeView::mousePressEvent(event);
    if( event->button() == Qt::LeftButton ) dragStartPosition = event->pos();
    return;
}

void AppTreeView::startDrag( Qt::DropActions supportedActions )
{
    QRect rect;
    QModelIndexList indexes;
    QMimeData *data;
    QDrag *drag;
    QSize rectSize;
    AppInstanceTree *app_inst_list;
    AppInstance *app_inst;
    AppClassTree *app_class_tree;
    // AppClassList *app_class_list;
    AppClass *app_class = NULL;

    if( model() )
    {
        indexes = selectedIndexes();
        if( indexes.count() > 0 )
        {
            data = model()->mimeData( indexes );
            if( data )
            {
                drag = new QDrag(this);
                app_inst_list = qobject_cast<AppInstanceTree *>(model());
                if( app_inst_list )
                {
                    app_inst = app_inst_list->itemFromIndex( indexes.first() );
                    if( app_inst )
                    {
                        rectSize = app_inst->size().toSize();
                    }
                }

                app_class_tree = qobject_cast<AppClassTree *>(model());
                if( app_class_tree )
                {
                    QStringList id_list = app_class_tree->itemFromIndex( indexes.first() )->data().toStringList();
                    if( !id_list.isEmpty() ) app_class =  AppLibrary::getClass( id_list.first() );
                    if( app_class )
                    {
                        rectSize = app_class->size();
                    }
                }

                if( mainWindow ) rectSize *= mainWindow->currentLayoutZoom;

#if 1
                if( !rectSize.isEmpty() )
                {
                    rectSize += QSize(1,1);
                    QPixmap pixmap( rectSize );
                    pixmap.fill( Qt::transparent );
                    QPainter painter( &pixmap );
                    QRect rect = pixmap.rect();
                    rect.setWidth( rect.width() - 1 );
                    rect.setHeight( rect.height() - 1 );
                    painter.drawRect( rect );
                    drag->setPixmap( pixmap );
                    drag->setHotSpot( QPoint(0,0) );
                }
#endif
                drag->setMimeData(data);
                drag->exec(supportedActions, Qt::CopyAction);
            }
        }
    }
    return;
}


void AppTreeView::keyPressEvent( QKeyEvent * event )
{
    if( event->key() == Qt::Key_Delete )
    {
        AppInstanceTree *app_inst_list = qobject_cast<AppInstanceTree *>(model());
        if( app_inst_list )
        {
            QModelIndexList list = selectedIndexes();
            foreach( QModelIndex idx, list )
            {
                AppInstance *app_inst = app_inst_list->itemFromIndex( idx );
                app_inst->keyPressEvent( new QKeyEvent(event->type(), event->key(), event->modifiers() ) );
            }
        }
        return;
    }
    QTreeView::keyPressEvent( event );
    return;
}
