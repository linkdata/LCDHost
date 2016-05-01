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

#ifndef EVENTCREATEAPPDEVICE_H
#define EVENTCREATEAPPDEVICE_H

#include "EventBase.h"
#include <QString>
#include <QByteArray>
#include <QSize>
#include <QImage>

#include "AppId.h"

// Sent from LibLibrary to AppLibrary to create an AppDevice for a
// newly arrived device.

class EventCreateAppDevice : public EventBase
{
public:
    QByteArray devid;
    QString name;
    QSize size;
    int depth;
    bool autoselect;
    QImage logo;

    EventCreateAppDevice( QByteArray di, QString n, QSize s, int d, bool a, const QImage & l = QImage() ) :
        EventBase( type() ), devid(di), name(n), size(s), depth(d), autoselect(a), logo(l)
    {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType(__FILE__)); return typeVal; }
};


#endif // EVENTCREATEAPPDEVICE_H
