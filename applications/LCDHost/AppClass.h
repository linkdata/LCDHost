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

#ifndef APPCLASS_H
#define APPCLASS_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QStringList>
#include <QUrl>
#include <QVariant>

class AppLibrary;

class AppClass : public QObject
{
    Q_OBJECT

    unsigned seqno_;
    QString name_;
    QSize size_;
    QStringList path_;

public:
    explicit AppClass(AppLibrary *lib, const QString &id, const QString &path, const QString &name, const QSize &size);
    ~AppClass();

    QString id() const { return objectName(); }
    QStringList path() const { return path_; }
    QString name() const { return name_; }
    QSize size() const { return size_; }
    AppLibrary *parent() const;
    QString generateName();

    static QUrl urlFromId(QString id) { return "lcdhostclass:"+id; }

    // TreeView stuff
    int columnCount() const { return 1; }
    QVariant data( int column, int role );
    static QVariant headerData( int section, Qt::Orientation orientation, int role );
};

#endif // APPCLASS_H
