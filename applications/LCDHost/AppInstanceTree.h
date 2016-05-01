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

#ifndef APPINSTANCETREE_H
#define APPINSTANCETREE_H

#include <QAbstractItemModel>
#include <QXmlStreamReader>
#include <QMimeData>
#include <QPointF>

class AppInstance;

class AppInstanceTree : public QAbstractItemModel
{
    Q_OBJECT

    bool loading_;
    AppInstance *root_;

public:
    explicit AppInstanceTree( QObject * parent = 0 );
    ~AppInstanceTree();

    // Mandatory:   index(), parent(), rowCount(), columnCount(), data()
    QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex & index ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent) const;
    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    // bool insertRows( int position, int rows, const QModelIndex &parent = QModelIndex() );
    // bool removeRows( int position, int rows, const QModelIndex &parent = QModelIndex() );
    Qt::ItemFlags flags ( const QModelIndex & index ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    
    // Utility
    AppInstance *itemFromIndex( const QModelIndex &index ) const;
    QModelIndex indexFromItem( const AppInstance *app_inst ) const;
    QList<AppInstance*> list() const;
    bool isEmpty() const;
    AppInstance *root() { return root_; }
    void createRoot();
    void destroyRoot();
    void refreshInstance( AppInstance * );

    // Insertion and removal
    bool insertAppInstance( AppInstance *app_inst, const QModelIndex & index = QModelIndex(), QPointF pos = QPointF(-999,-999) );
    bool insertAppInstance( AppInstance *app_inst, AppInstance *parent_inst ) { return insertAppInstance( app_inst, indexFromItem(parent_inst) ); }
    AppInstance *takeAppInstance( const QModelIndex & index );
    bool removeAppInstance( AppInstance *app_inst );

    // Z ordering
    bool zUp( AppInstance *app_inst );
    bool zDown( AppInstance *app_inst );
    void sortByZ( AppInstance *parent );

    // Drag & drop
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes () const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );

    bool load( QXmlStreamReader & stream );
    void save( QXmlStreamWriter & stream );

    void refresh() { beginResetModel(); endResetModel(); }
    void deviceChanged( int w, int h, int d );
};


#endif // APPINSTANCELIST_H
