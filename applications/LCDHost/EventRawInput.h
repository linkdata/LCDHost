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

#ifndef EVENTRAWINPUT_H
#define EVENTRAWINPUT_H

#include "EventBase.h"
#include <QString>
#include "AppId.h"

// Used to send input events, both from devices to the application for broadcast
// and from AppObjects to LibObjects when they're applicable

class EventRawInput : public EventBase
{
public:
    enum Flags
    {
        Button      = 0x0001,
        Value       = 0x0002,
        Down        = 0x0004, // button down or positive values
        Pos         = 0x0004,
        Up          = 0x0008, // button up or negative values
        Neg         = 0x0008,
        Keyboard    = 0x1000,
        Mouse       = 0x2000,
        Joystick    = 0x4000,
        Other       = 0x8000
    };

    enum MouseItem
    {
        X, Y, Wheel
    };

    AppId receiver;
    QByteArray devid;
    QString control;
    int item;
    int value;
    int flags;

    bool operator==( const EventRawInput &other ) const { return (devid == other.devid) && (item == other.item); }
    bool operator==( const EventRawInput *other ) const { if( other ) return (devid == other->devid) && (item == other->item); else return false; }

    EventRawInput() :
            EventBase( type() ),
            receiver(AppId()),
            devid(QByteArray()),
            item(0),
            value(0),
            flags(0)
    {
    }

    EventRawInput( const EventRawInput& r ) :
            EventBase( type() ),
            receiver(r.receiver),
            devid(r.devid),
            control(r.control),
            item(r.item),
            value(r.value),
            flags(r.flags)
    {
    }

    EventRawInput( AppId r, QByteArray d, QString c, int i, int v, int f ) :
            EventBase( type() ),
            receiver(r),
            devid(d),
            control(c),
            item(i),
            value(v),
            flags(f)
    {
    }

    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType(__FILE__)); return typeVal; }

    static QString encodeControl(QString d, int i, int f )
    {
        QString control = d;
        if( f & Button ) control += '.';
        else control += '/';
        control += QString::number(i,16);
        return control;
    }
};

#endif // EVENTRAWINPUT_H
