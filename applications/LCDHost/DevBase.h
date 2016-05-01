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

#ifndef DEVBASE_H
#define DEVBASE_H

#include <QThread>
#include <QImage>
#include <QList>
#include <QMap>

#include "DevId.h"

class DevBase : public QThread
{
    Q_OBJECT

public:
    DevBase() : QThread() {}
    virtual ~DevBase() {}

    virtual QList<DevId> list() = 0;
    virtual QSize size( DevId ) = 0;
    virtual bool monochrome( DevId ) = 0;
    virtual const QMap<int,QString>& buttons( DevId ) = 0;
    virtual void open( DevId ) = 0;
    virtual void render( DevId, QImage ) = 0;
    virtual void close( DevId ) = 0;

    static void init();
    static void term();
    static QList<DevId> deviceList();

public slots:
    virtual void setImage( QImage );

signals:
    void buttonDown( int id );      // sent on button down
    void buttonClick( int id );     // sent on button up before longclick time
    void buttonLongClick( int id ); // sent on button still down after longclick time
    void buttonUp( int id );        // sent on button up

private:
    static QList<DevBase*> drivers;
};

#endif // DEVBASE_H
