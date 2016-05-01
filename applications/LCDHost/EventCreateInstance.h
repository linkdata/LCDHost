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

#ifndef EVENTCREATEINSTANCE_H
#define EVENTCREATEINSTANCE_H

#include "EventBase.h"
#include <QString>
#include "AppId.h"

// Sent from AppLibrary to LibLibrary to create a new LibInstance

class EventCreateInstance : public EventBase
{
public:
    QString class_id;
    AppId inst_id;
    QString obj_name;

    EventCreateInstance( QString c, AppId i, QString s ) : EventBase( type() ), class_id(c), inst_id(i), obj_name(s) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType(__FILE__)); return typeVal; }
};

#endif // EVENTCREATEINSTANCE_H
