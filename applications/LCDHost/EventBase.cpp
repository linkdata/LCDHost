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


#include <QMap>
#include <QFileInfo>
#include "EventBase.h"

QMap<int,QString> *EventBase::lcdhost_event_names_ = 0;

QString EventBase::name( int t )
{
    if( t < QEvent::User || t > QEvent::MaxUser ) return QString();
    if( lcdhost_event_names_ == 0 ) lcdhost_event_names_ = new QMap<int,QString>();
    if( lcdhost_event_names_->contains(t) ) return lcdhost_event_names_->value( t );
    return QString::number(t);
}

int EventBase::registerEventType( const char *filename, int hint )
{
    int typeVal = QEvent::registerEventType(hint);
    if( lcdhost_event_names_ == 0 ) lcdhost_event_names_ = new QMap<int,QString>();
    QString name = QFileInfo(filename).baseName();
    lcdhost_event_names_->insert( typeVal, name );
    return typeVal;
};
