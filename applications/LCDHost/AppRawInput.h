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

#ifndef APPRAWINPUT_H
#define APPRAWINPUT_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QAbstractListModel>

#include "EventRawInput.h"

// #define AppInput AppRawInput::instance()

class RawInputEngine;
class RawInputDevice;

class AppRawInput : public QAbstractListModel
{
  Q_OBJECT

  static AppRawInput* instance_;

  QMutex monitormutex_;
  QList<QObject*> monitorlist_;
  QList<QString> monitordevs_;
  bool isrefreshing_;
  int listeners_;
  RawInputEngine *e;

  explicit AppRawInput();

public:
  // static AppRawInput& instance() { static AppRawInput instance_; return instance_; }
  static void CreateInstance();
  static AppRawInput* instance() { return instance_; }
  static void DestroyInstance();

  ~AppRawInput();

  void clear();

  // Mandatory:   rowCount(), data()
  int rowCount( const QModelIndex &parent = QModelIndex() ) const;
  QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
  int columnCount( const QModelIndex &parent) const;
  Qt::ItemFlags flags ( const QModelIndex & index ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  bool event( QEvent *e );

  QStringList getCapturedList();
  void setCapturedList( QStringList list );

  QString describeEvent( EventRawInput *e ) { return describeEvent( e->devid, e->control, e->item, e->value, e->flags ); }
  QString describeEvent( QByteArray devid, QString control, int item, int val, int flags );
  QString describeControl( EventRawInput *e ) { return describeControl( e->devid, e->control, e->item, e->value, e->flags ); }
  QString describeControl( QByteArray devid, QString control, int item, int val, int flags );
  RawInputDevice device(int n);
  RawInputDevice device(QByteArray devid);
  QString itemName( EventRawInput *e ) { return itemName( e->devid, e->control, e->item, e->value, e->flags ); }
  QString itemName( QByteArray devid, QString control, int item, int val, int flags );

  void broadcast( EventRawInput *evt );

signals:
  void rawInput( QByteArray devid, QString control, int item, int val, int flags );

public slots:
  void refresh();
};

#endif // APPRAWINPUT_H
