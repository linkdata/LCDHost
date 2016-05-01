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

#ifndef EVENTLGLCDNOTIFICATION_H
#define EVENTLGLCDNOTIFICATION_H

#include "QEvent"

class EventLgLcdNotification : public QEvent
{
public:
    int code;
    int index;
    int p1;

    EventLgLcdNotification( int a_code, int i, int a_p1 = 0 ) : QEvent( type() ), code(a_code), index(i), p1(a_p1) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType()); return typeVal; }
};

#endif // EVENTLGLCDNOTIFICATION_H
