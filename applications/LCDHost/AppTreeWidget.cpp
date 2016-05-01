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

#include "AppTreeWidget.h"
#include "AppObject.h"
#include "AppInstance.h"
#include "AppSetupItem.h"

#include <QApplication>

QTreeWidgetItem * AppTreeWidget::makeChild( QTreeWidgetItem * parent, const QString & name )
{
    QTreeWidgetItem * child = findChild( parent, name );
    if( child ) return child;
    child = new QTreeWidgetItem();
    child->setText(0,name);
    child->setFlags( Qt::ItemIsEnabled );
    child->setForeground( 0, QApplication::palette().brush(QPalette::Disabled,QPalette::Foreground) );
    if( parent ) parent->addChild( child );
    else addTopLevelItem( child );
    return child;
}

QTreeWidgetItem * AppTreeWidget::findChild( const QTreeWidgetItem * parent, const QString & name ) const
{
    if( parent )
    {
        for( int i = 0; i < parent->childCount(); ++ i )
            if( parent->child(i)->text(0) == name ) return parent->child(i);
        return 0;
    }

    for( int i = 0; i < topLevelItemCount(); ++ i )
        if( topLevelItem(i)->text(0) == name ) return topLevelItem(i);

    return 0;
}

QTreeWidgetItem * AppTreeWidget::addItem( const QStringList & path, int which, QTreeWidgetItem * where )
{
    if( which < 0 || which >= path.size() ) return where;
    return addItem( path, which+1, makeChild( where, path.at(which) ) );
}

QTreeWidgetItem * AppTreeWidget::addItem( const QString & ui_path, const QString & link_path )
{
    QTreeWidgetItem * item = addItem( ui_path.split('/',QString::SkipEmptyParts), 0, 0 );
    if( item )
    {
        item->setData( 0, Qt::ToolTipRole, link_path );
        if( !link_path.isEmpty() )
        {
            item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
            item->setForeground( 0, QApplication::palette().brush(QPalette::Foreground) );
        }
        item->setText( 1, link_path );
    }
    return item;
}

QTreeWidgetItem * AppTreeWidget::addItem( AppSetupItem * asi )
{
    if( asi && asi->parent() && asi->isVerified() )
    {
        if( !asi->publishPath().isEmpty() )
            return addItem( asi->publishPath(), asi->publishPath() );
        return addItem( asi->path(), asi->path() );
    }

    return 0;
}

bool AppTreeWidget::setCurrentItemByLinkPath( const QString & path )
{
    QList<QTreeWidgetItem *> list( findItems( path, Qt::MatchFixedString|Qt::MatchRecursive, 1 ) );
    if( list.isEmpty() ) return false;
    scrollToItem( list.first() );
    setCurrentItem( list.first() );
    return true;
}
