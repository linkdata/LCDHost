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

#ifndef APPCLASSTREE_H
#define APPCLASSTREE_H

#include <QStandardItemModel>
#include <QStringList>

class AppClass;
class MainWindow;

class AppClassTree : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit AppClassTree(MainWindow *parent);
    MainWindow* mainWindow() const;
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes () const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );

public slots:
    void insertClass( QStringList path, QString id );
    void removeClass( QStringList path, QString id );

private:
    void insertAt( QStandardItem *parent, QStringList path, QString id );
};

#endif // APPCLASSTREE_H
