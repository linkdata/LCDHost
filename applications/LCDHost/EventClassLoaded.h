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

#ifndef EVENTCLASSLOADED_H
#define EVENTCLASSLOADED_H

#include "EventBase.h"
#include <QString>
#include <QSize>

// Sent by LibLibrary when a new LibClass has been created
class EventClassLoaded : public EventBase
{
public:
    QString id;
    QString path;
    QString name;
    QSize size;
    EventClassLoaded( QString i, QString p, QString n, QSize s ) : EventBase( type() ), id(i), path(p), name(n), size(s) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType(__FILE__)); return typeVal; }
};

#endif // EVENTCLASSLOADED_H
