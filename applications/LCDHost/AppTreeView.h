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

#ifndef APPTREEVIEW_H
#define APPTREEVIEW_H

#include <QtDebug>
#include <QTreeView>

class AppTreeView : public QTreeView
{
    Q_OBJECT

public:
    AppTreeView( QWidget * parent = 0 ) : QTreeView( parent ) {}

protected:
    void mousePressEvent( QMouseEvent * event );
    void startDrag( Qt::DropActions supportedActions );
    void keyPressEvent( QKeyEvent * event );

protected slots:
    void selectionChanged( const QItemSelection & selected, const QItemSelection & deselected );
    void currentChanged( const QModelIndex & current, const QModelIndex & previous );

signals:
    void selectionChange( const QItemSelection & selected, const QItemSelection & deselected );
    void wantPreview( QString );

private:
    QPoint dragStartPosition;

};

#endif // APPLISTVIEW_H
