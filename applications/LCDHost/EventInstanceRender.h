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

#ifndef EVENTINSTANCERENDER_H
#define EVENTINSTANCERENDER_H

#include "EventBase.h"
#include <QImage>
#include <QSize>

#include "AppId.h"

// Sent from AppInstance to LibInstance to request a rendering
class EventInstanceRender : public EventBase
{
public:
    AppId receiver;
    int width, w_method, w_mod;
    int height, h_method, h_mod;
    EventInstanceRender( AppId r, int w, int wme, int wmo, int h, int hme, int hmo )
        : EventBase( type() ),
        receiver(r),
        width(w), w_method(wme), w_mod(wmo),
        height(h), h_method(hme), h_mod(hmo) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType(__FILE__)); return typeVal; }
};


#endif // EVENTINSTANCERENDER_H
