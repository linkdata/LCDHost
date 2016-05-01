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

#ifndef EVENTSETUPITEM_H
#define EVENTSETUPITEM_H

#include "EventBase.h"

#include "AppId.h"
#include "AppSetupItem.h"
#include "LibSetupItem.h"

// Sent by LibObject to AppObject to inform about a setup item from the plugin
class EventSetupItem : public EventBase
{
public:
    LibSetupItem lsi;

    EventSetupItem( const lh_setup_item* i ) : EventBase( type() ), lsi(i) { lsi.checkDefaults(); }
    EventSetupItem( const AppSetupItem* a ) : EventBase( type() ), lsi(a) {}
    static QEvent::Type type() { static QEvent::Type typeVal = static_cast<QEvent::Type>(registerEventType(__FILE__)); return typeVal; }
};

#endif // EVENTSETUPITEM_H
