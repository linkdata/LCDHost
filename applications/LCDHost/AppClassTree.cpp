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


#include <QMimeData>
#include <QList>
#include <QUrl>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppClassTree.h"
#include "AppClass.h"
#include "AppInstanceTree.h"
#include "AppInstance.h"

AppClassTree::AppClassTree(MainWindow *parent)
    : QStandardItemModel(parent)
{
    setHorizontalHeaderLabels(QStringList("Name"));
    return;
}

MainWindow* AppClassTree::mainWindow() const
{
    return static_cast<MainWindow*>(QObject::parent());
}

void AppClassTree::insertAt( QStandardItem *parent, QStringList path, QString id )
{
    QStandardItem *item = NULL;
    QStringList id_list;
    QString str;

    str = path.takeFirst();
    for( int row = 0; row < parent->rowCount(); row ++ )
    {
        if( parent->child(row)->text().compare( str, Qt::CaseInsensitive ) == 0 )
        {
            item = parent->child(row);
            break;
        }
    }

    if( item == NULL )
    {
        item = new QStandardItem(str);
        // item->setEditable( false );
        // item->setEnabled( true );
        parent->appendRow( item );
        parent->sortChildren(0,Qt::AscendingOrder);
    }

    if( !path.isEmpty() )
    {
        insertAt( item, path, id );
        return;
    }

    id_list = item->data().toStringList();
    if( id_list.contains( id ) ) return;

    id_list.append( id );
    item->setData( id_list );

    if( id_list.size() > 1 )
    {
        item->setToolTip( tr("%n items",0,id_list.size()) );
        // item->setEnabled( true );
        foreach( QString sub_id, id_list )
            insertAt( item, QStringList(sub_id), sub_id );
    }
    else
    {
        item->setToolTip( id );
        // item->setEnabled( true );
    }

    return;
}

void AppClassTree::insertClass( QStringList path, QString id )
{
    insertAt( invisibleRootItem(), path, id );
    return;
}

void AppClassTree::removeClass( QStringList path, QString id )
{
    Q_UNUSED(path);
    Q_UNUSED(id);
    return;
}

Qt::DropActions AppClassTree::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList AppClassTree::mimeTypes () const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

// Drop the preview item if it correct class
QMimeData *AppClassTree::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> url_list;
    QModelIndex index;
    QStringList id_list;

    foreach(index, indexes)
    {
        if (index.isValid())
        {
            id_list = itemFromIndex( index )->data().toStringList();
            if( !id_list.isEmpty() )
            {
                url_list << AppClass::urlFromId( id_list.first() );
            }
        }
    }

    mimeData->setUrls( url_list );
    return mimeData;
}

bool AppClassTree::dropMimeData( const QMimeData * data, Qt::DropAction action, int, int column, const QModelIndex & )
{
    AppInstance *app_inst;

    if( action == Qt::IgnoreAction ) return true;
    if( column > 0 ) return false;

    if( data->hasUrls() )
    {
        foreach( QUrl url, data->urls() )
        {
            if( url.scheme() == "lcdhostobject" )
            {
                app_inst = AppInstance::fromId( AppId::fromString( url.path() ) );
                if(app_inst && app_inst->isModifiable() && app_inst->parentItem())
                {
                    layoutModified();
                    mainWindow()->tree()->removeAppInstance( app_inst );
                    app_inst->term();
                    delete app_inst;
                }
            }
        }
    }

    return true;
}

