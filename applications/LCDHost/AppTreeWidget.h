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

#ifndef APPTREEWIDGET_H
#define APPTREEWIDGET_H

#include <QTreeWidget>

class AppObject;
class AppSetupItem;

class AppTreeWidget : public QTreeWidget
{
    Q_OBJECT

    QTreeWidgetItem * addItem( const QStringList & path, int which, QTreeWidgetItem * where );

public:
    explicit AppTreeWidget( QWidget *parent = 0 ) :
        QTreeWidget( parent )
    {
        setColumnCount(2);
        setColumnHidden(1,true);
    }

    QTreeWidgetItem * findChild( const QTreeWidgetItem * parent, const QString & name ) const;
    QTreeWidgetItem * makeChild( QTreeWidgetItem * parent, const QString & name );
    QTreeWidgetItem * addItem( const QString & ui_path, const QString & link_path );
    QTreeWidgetItem * addItem( AppSetupItem * asi );

    bool setCurrentItemByLinkPath( const QString & path );
};

#endif // APPTREEWIDGET_H
