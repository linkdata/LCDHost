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

#ifndef DEVID_H
#define DEVID_H

#include <QObject>
#include <QEvent>
#include <QString>
#include <QSize>
#include <QVariant>
#include <QImage>

class DevBase;

class DevId
{
public:
    QString name;
    DevBase *drv;
    void *d;

    DevId() : name(QString()), drv(NULL), d(NULL) {}
    DevId( QString s, DevBase *dev, void *p ) : name(s), drv(dev), d(p) {}
    DevId( const DevId& r ) : name(r.name), drv(r.drv), d(r.d) {}
    DevId& operator=(const DevId& r) { if( this != &r ) { name=r.name; drv=r.drv; d=r.d; } return *this; }
    bool operator==( const DevId &other ) const { return (drv == other.drv) && (d == other.d); }

    ~DevId() {}

    bool valid() const { return (drv!=NULL && d!=NULL); }
    void clear() { name=QString(); drv=NULL; d=NULL; }

    void open() const;
    void render( QImage img ) const;
    void close() const;

    QSize size() const;
    bool monochrome() const;
    const QMap<int,QString>& buttons() const;
};

Q_DECLARE_METATYPE(DevId)

QDataStream & operator<< (QDataStream& stream, const DevId& id);
QDataStream & operator>> (QDataStream& stream, DevId& id);

class EventDevOpen : public QEvent
{
public:
    DevId devid;

    EventDevOpen( DevId id ) : QEvent( type() ), devid(id) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType()); return typeVal; }
};

class EventDevRender : public QEvent
{
public:
    DevId devid;
    QImage image;

    EventDevRender( DevId id, QImage img ) : QEvent( type() ), devid(id), image(img) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType()); return typeVal; }
};

class EventDevClose : public QEvent
{
public:
    DevId devid;

    EventDevClose( DevId id ) : QEvent( type() ), devid(id) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType()); return typeVal; }
};

#endif // DEVID_H
