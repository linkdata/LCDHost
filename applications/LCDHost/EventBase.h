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

#ifndef EVENTBASE_H
#define EVENTBASE_H

#include <QEvent>
#include <QString>
#include <QMap>

class EventBase : public QEvent
{
    static QMap<int,QString> *lcdhost_event_names_;

public:
    explicit EventBase( Type type ) : QEvent(type) {}

    static QString name( int t );
    static int registerEventType( const char *filename, int hint = -1 );
};

#endif // EVENTBASE_H
