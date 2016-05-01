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
// #include <QKeyEvent>
#include <QUrl>

#include "LCDHost.h"
#include "MainWindow.h"
#include "AppDevice.h"
#include "AppInstance.h"
#include "AppInstanceTree.h"

AppInstanceTree::AppInstanceTree( QObject * parent)
    : QAbstractItemModel(parent)
    , loading_(false)
    , root_(0)
{
    return;
}

AppInstanceTree::~AppInstanceTree()
{
    // rootItem is deleted by the scene
    if(root_)
    {
        root_->term();
        root_ = 0;
    }
    loading_ = false;
    return;
}

void AppInstanceTree::createRoot()
{
    Q_ASSERT( root_ == NULL );
    Q_ASSERT( mainWindow != NULL );
    Q_ASSERT( mainWindow->scene() != NULL );
    Q_ASSERT( mainWindow->scene()->items().size() == 0 );
    root_ = new AppInstance();
    root_->init();
    root_->setModifiable( false );
    root_->setSelectable( false );
    root_->setObjectName( AppDevice::current().name() );
    root_->setFlag( QGraphicsItem::ItemIsSelectable, false );
    root_->setFlag( QGraphicsItem::ItemIsMovable, false );
    root_->setPos( 0, 0 );
    root_->setSize( AppDevice::current().size() );
    mainWindow->scene()->addItem( root_ );
    return;
}

void AppInstanceTree::destroyRoot()
{
    if( root_ )
    {
        mainWindow->scene()->removeItem( root_ );
        root_->term();
        delete root_;
        root_ = 0;
    }
}

void AppInstanceTree::refreshInstance( AppInstance *app_inst )
{
    emit dataChanged( indexFromItem(app_inst), indexFromItem(app_inst) );
    return;
}

QModelIndex AppInstanceTree::index( int row, int column, const QModelIndex & parent ) const
{
    if( row < 0 || column < 0 ) return QModelIndex();
    if( parent.isValid() && parent.column() != 0 ) return QModelIndex();
    AppInstance *parentItem = itemFromIndex( parent );
    if( parentItem == NULL ) return QModelIndex();

    QList<QGraphicsItem*> kids = parentItem->childItems();
    if( row >= kids.size() )
    {
        return QModelIndex();
    }
    QList<QGraphicsItem*>::const_iterator i;
    for( i = kids.constBegin(); i != kids.constEnd(); ++i )
    {
        if( static_cast<AppInstance*>(*i)->zValue() == row )
        {
            return createIndex( row, column, static_cast<AppInstance*>(*i) );
        }
    }
    Q_ASSERT(0);
    return QModelIndex();
    // qDebug() << "AppInstanceTree::index("<<row<<column<<parent<<") "<< kids.at(row)->name() <<" = ["<< parentItem->name() << kids.size() <<"]" << result;
}

QModelIndex AppInstanceTree::parent( const QModelIndex & index ) const
{
    if( !index.isValid() ) return QModelIndex();
    AppInstance *childItem = itemFromIndex( index );
    if( childItem == NULL ) return QModelIndex();
    AppInstance *parentItem = childItem->parentItem();
    if( parentItem == NULL || parentItem == root_ ) return QModelIndex();
    Q_ASSERT( parentItem ? ( parentItem->childItemsByZ().indexOf(childItem) == (int) childItem->zValue() ) : true );
    return createIndex( (int) parentItem->zValue(), 0, parentItem );
}

int AppInstanceTree::rowCount( const QModelIndex &index ) const
{
    AppInstance *app_inst;
    if( !index.isValid() )
    {
        if( root_ == NULL ) return 0;
        return root_->childItems().size();
    }
    if( index.column() != 0 ) return 0;
    app_inst = itemFromIndex( index );
    if( app_inst == NULL ) return 0;
    return app_inst->childItems().size();
}

int AppInstanceTree::columnCount( const QModelIndex &parent ) const
{
    if( parent.isValid() )
    {
        AppInstance *app_inst = itemFromIndex( parent );
        if( app_inst == NULL ) return 0;
        return app_inst->columnCount();
    }
    if( root_ ) return root_->columnCount();
    return 0;
}

QVariant AppInstanceTree::data(const QModelIndex &index, int role) const
{
    if( !index.isValid() ) return QVariant();
    AppInstance *app_inst = itemFromIndex( index );
    if( app_inst == NULL ) return QVariant();
    return app_inst->data( index.column(), role );
}

Qt::ItemFlags AppInstanceTree::flags( const QModelIndex & index ) const
{
    Qt::ItemFlags f = Qt::ItemIsDropEnabled;

    if( index.isValid() )
    {
        AppInstance *app_inst = itemFromIndex( index );
        if( app_inst )
        {
            if( app_inst->isModifiable() ) f |= Qt::ItemIsDragEnabled;
            f |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
    }

    return f;
}

QVariant AppInstanceTree::headerData( int section, Qt::Orientation orientation, int role ) const
{
    return AppInstance::headerData( section, orientation, role );
}

////////////////////////////////////////////////////////////////////////
//
// Insertion and removal
//
////////////////////////////////////////////////////////////////////////

bool AppInstanceTree::insertAppInstance( AppInstance *app_inst, const QModelIndex & index, QPointF pos )
{
    QModelIndex parent_index;
    AppInstance *parent_inst = root_;
    int target_z;

    Q_ASSERT(root_);
    if(index.isValid()) {
        parent_inst = itemFromIndex(index);
        parent_index = index;
        Q_ASSERT(parent_inst);
        Q_ASSERT(!parent_inst->hasParentItem(app_inst));
    }

    Q_ASSERT(indexFromItem(parent_inst) == parent_index);

    if(!parent_inst || parent_inst->hasParentItem(app_inst))
        return false;

    if( pos == QPointF(-999,-999) ) pos = app_inst->scenePos();
    if( pos == QPointF(-999,-999) ) pos = parent_inst->scenePos();

    if( app_inst->zValue() >= 0 ) target_z = app_inst->zValue();
    else target_z = parent_inst->childItems().size();

    beginInsertRows(parent_index, target_z, target_z);
    app_inst->setParentItem(parent_inst);
    app_inst->setScenePos(pos);
    app_inst->setZValue(target_z);
    // qDebug() << "AppInstanceTree::insertAppInstance()" << app_inst->objectName() << app_inst->zValue() << app_inst->pos();
    endInsertRows();
    return true;
}

AppInstance *AppInstanceTree::takeAppInstance( const QModelIndex & index )
{
    AppInstance *app_inst;
    if( !index.isValid() ) return NULL;
    app_inst = itemFromIndex( index );
    if( app_inst && !removeAppInstance( app_inst ) ) return NULL;
    return app_inst;
}

bool AppInstanceTree::removeAppInstance( AppInstance *app_inst )
{
    AppInstance *parent;
    int position;

    parent = app_inst->parentItem();
    if( parent == NULL ) parent = root_;
    if( parent == NULL ) return false;

    QList<AppInstance *> siblings = parent->childItemsByZ();
    position = (int) app_inst->zValue();
    Q_ASSERT( position >= 0 );
    Q_ASSERT( position == siblings.indexOf( app_inst ) );

    if( position >= 0 )
    {
        beginRemoveRows( indexFromItem(parent), position, position );
        app_inst->setParentItem( NULL );
        app_inst->setZValue(-1);
        while( ++position < siblings.size() )
        {
            Q_ASSERT( static_cast<int>(siblings.at( position )->zValue()) == position );
            siblings.at( position )->setZValue( position-1 );
        }
        endRemoveRows();
        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////
//
// Z ordering
//
////////////////////////////////////////////////////////////////////////

void AppInstanceTree::sortByZ( AppInstance *parent )
{
    AppInstance *prev_inst = NULL;
    AppInstance *curr_inst = NULL;
    foreach( QGraphicsItem *gitem, parent->childItems() )
    {
        curr_inst = static_cast<AppInstance *>(gitem);
        if( curr_inst )
        {
            sortByZ( curr_inst );
            if( prev_inst )
            {
                qreal curr_z = curr_inst->zValue();
                qreal prev_z = prev_inst->zValue();
                if( prev_z > curr_z )
                {
                    layoutAboutToBeChanged();
                    curr_inst->setZValue( prev_z );
                    changePersistentIndex( createIndex( curr_z, 0, (void*) curr_inst ), createIndex( prev_z, 0, (void*) curr_inst ) );
                    prev_inst->setZValue( curr_z );
                    changePersistentIndex( createIndex( prev_z, 0, (void*) prev_inst ), createIndex( curr_z, 0, (void*) prev_inst ) );
                    layoutChanged();
                }
            }
        }
        prev_inst = curr_inst;
    }
}

bool AppInstanceTree::zUp( AppInstance *app_inst )
{
    int the_z;
    AppInstance *app_other, *parent;

    parent = app_inst->parentItem();
    if( parent == NULL ) parent = root_;
    if( parent == NULL ) return false;

    the_z = (int) app_inst->zValue();

    if( the_z >= parent->childItems().size()-1 ) return false; // can't go higher

    app_other = parent->childItemsByZ().at( the_z + 1 );
    if( app_other == NULL ) return false;

    layoutAboutToBeChanged();
    app_other->setZValue( the_z );
    app_inst->setZValue( the_z + 1);
    changePersistentIndex( createIndex( the_z, 0, (void*) app_inst ), createIndex( the_z+1, 0, (void*) app_inst ) );
    changePersistentIndex( createIndex( the_z+1, 0, (void*) app_other ), createIndex( the_z, 0, (void*) app_other ) );
    layoutChanged();
    return true;
}

bool AppInstanceTree::zDown( AppInstance *app_inst )
{
    int the_z;
    AppInstance *app_other, *parent;

    if( app_inst->zValue() <= 0 ) return false; // can't go lower

    parent = app_inst->parentItem();
    if( parent == NULL ) parent = root_;
    if( parent == NULL ) return false;

    the_z = (int) app_inst->zValue();

    app_other = parent->childItemsByZ().at( the_z - 1 );
    if( app_other == NULL ) return false;

    layoutAboutToBeChanged();
    app_inst->setZValue( the_z-1);
    app_other->setZValue( the_z );
    changePersistentIndex( createIndex( the_z, 0, (void*) app_inst ), createIndex( the_z-1, 0, (void*) app_inst ) );
    changePersistentIndex( createIndex( the_z-1, 0, (void*) app_other ), createIndex( the_z, 0, (void*) app_other ) );
    layoutChanged();
    return true;
}

////////////////////////////////////////////////////////////////////////
//
// Utility
//
////////////////////////////////////////////////////////////////////////

AppInstance *AppInstanceTree::itemFromIndex( const QModelIndex &index ) const
{
    AppInstance *app_inst;
    if( !index.isValid() ) return root_;
    app_inst = static_cast<AppInstance *>( index.internalPointer() );
#ifndef QT_NO_DEBUG
    AppInstance *parent_inst = app_inst->parentItem();
    if( parent_inst )
    {
        QList<AppInstance*> kids = parent_inst->childItemsByZ();
        int i = kids.indexOf(app_inst);
        Q_ASSERT( i == (int) app_inst->zValue() );
    }
#endif
    return app_inst;
}

QModelIndex AppInstanceTree::indexFromItem( const AppInstance *app_inst ) const
{
    if( app_inst == root_ ) return QModelIndex();
#ifndef QT_NO_DEBUG
    AppInstance *parent_inst = app_inst->parentItem();
    if( parent_inst )
    {
        QList<AppInstance*> kids = parent_inst->childItemsByZ();
        int i = kids.indexOf(const_cast<AppInstance*>(app_inst));
        Q_ASSERT( i == (int) app_inst->zValue() );
    }
#endif
    return createIndex( (int) app_inst->zValue(), 0, (void*) app_inst );
}

QList<AppInstance*> AppInstanceTree::list() const
{
    QList<AppInstance*> theList;
    if( root_ ) root_->allChildren( theList );
    return theList;
}

bool AppInstanceTree::isEmpty() const
{
    if( root_ ) return root_->childItems().isEmpty();
    return true;
}

////////////////////////////////////////////////////////////////////////
//
// Drag & drop
//
////////////////////////////////////////////////////////////////////////

Qt::DropActions AppInstanceTree::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList AppInstanceTree::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list";
    return types;
}

QMimeData *AppInstanceTree::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> url_list;

    foreach (QModelIndex index, indexes)
    {
        if( index.isValid() )
        {
            AppInstance *app_inst = itemFromIndex( index );
            if( app_inst ) url_list << app_inst->url();
        }
    }
    mimeData->setUrls( url_list );
    return mimeData;
}

// This operation will change the Z order
bool AppInstanceTree::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & index )
{
    Q_UNUSED(row);

    if( action == Qt::IgnoreAction ) return true;
    if( column > 0 ) return false;

    if( data->hasUrls() )
    {
        AppInstance *target_inst;
        AppInstance *app_inst;

        target_inst = itemFromIndex( index );
        Q_ASSERT( indexFromItem(target_inst).row() == index.row() );

        foreach( QUrl url, data->urls() )
        {
            if( url.scheme() == "lcdhostobject" )
            {
                AppId theid = AppId::fromString( url.path() );
                app_inst = qobject_cast<AppInstance*>( theid.appObject() );
                if( app_inst && app_inst != target_inst && !target_inst->hasParentItem(app_inst) )
                {
                    QPointF old_pos;
                    bool old_selected = app_inst->isSelected();
                    layoutModified();

                    // dropped from preview? if so, make a copy, else remove from old pos
                    if( app_inst->parentItem() )
                    {
                        old_pos = app_inst->scenePos();
                        removeAppInstance( app_inst );
                    }
                    else
                    {
                        old_pos = QPointF(0,0);
                        app_inst = new AppInstance(*app_inst);
                        app_inst->init();
                        app_inst->setupComplete();
                    }

                    if( !insertAppInstance( app_inst, index, old_pos ) )
                    {
                        // eeek!
                        insertAppInstance( app_inst );
                    }
                    app_inst->setSelected( old_selected );
                }
            }

            if( url.scheme() == "lcdhostclass" )
            {
                AppClass *app_class;
                app_class =  AppLibrary::getClass( url.path() );
                if( app_class )
                {
                    AppInstance *app_inst = new AppInstance( app_class );
                    app_inst->init();
                    app_inst->setupComplete();
                    if( !insertAppInstance( app_inst, index ) )
                    {
                        app_inst->term();
                        delete app_inst;
                        app_inst = NULL;
                    }
                    else layoutModified();
                }
            }
        }
    }

    return true;
}

bool AppInstanceTree::load( QXmlStreamReader & stream )
{
    bool retv = false;
    QMap<AppInstance*,AppInstance*> load_id_map;

    Q_ASSERT( root_ != NULL );

    loading_ = true;
    while( root_ && !stream.atEnd() )
    {
        stream.readNext();
        if( stream.isStartElement() )
        {
            if( lh_log_load )
                qDebug("<span style=\"background-color: #f0f0f0;\">Loading</span> Starting");

            retv = root_->load( stream );

            if( lh_log_load )
                qDebug() << QString("<span style=\"background-color: #f0f0f0;\">%1</span> XML parsing complete")
                    .arg(stream.lineNumber(),5,10,QChar('0'));

            if( !retv )
            {
                qWarning() << "failed to load layout completely - blanking it";
                foreach( AppInstance *app_inst, root_->childItemsByZ() )
                {
                    app_inst->term();
                    delete app_inst;
                }
                loading_ = false;
                return false;
            }

            if( lh_log_load ) qDebug("<span style=\"background-color: #f0f0f0;\">Loading</span> ID map");
            root_->loadMap( load_id_map );

            if( lh_log_load ) qDebug("<span style=\"background-color: #f0f0f0;\">Loading</span> Tree");
            root_->loadTree( load_id_map );

            if( lh_log_load ) qDebug("<span style=\"background-color: #f0f0f0;\">Loading</span> References");
            root_->loadRefs( load_id_map );

            if( lh_log_load ) qDebug("<span style=\"background-color: #f0f0f0;\">Loading</span> Complete");
            break;
        }
    }
    loading_ = false;

    return retv;
}

void AppInstanceTree::save( QXmlStreamWriter & stream )
{
    stream.writeStartDocument();
    if( root_ ) root_->save( stream );
    stream.writeEndDocument();
}

void AppInstanceTree::deviceChanged( int w, int h, int d )
{
    Q_UNUSED(w);
    Q_UNUSED(h);
    Q_UNUSED(d);
    if( root_ )
    {
        root_->setSize( AppDevice::current().size() );
        root_->setObjectName( AppDevice::current().name() );
        root_->xwCalculate();
        root_->yhCalculate();
    }
}
