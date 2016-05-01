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

#ifndef EVENTCALLBACK_H
#define EVENTCALLBACK_H

#include "EventBase.h"
#include "lh_plugin.h"

// Posted by the plugin thread callback function to the LibLibrary,
// for forwarding to the appropriate LibObject
class EventCallback : public EventBase
{
public:
    const void *obj;
    lh_callbackcode code;
    void *param;
    EventCallback( const void* o, lh_callbackcode c, void *p ) : EventBase( type() ), obj(o), code(c), param(p) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType(__FILE__)); return typeVal; }
};

#endif // EVENTCALLBACK_H
