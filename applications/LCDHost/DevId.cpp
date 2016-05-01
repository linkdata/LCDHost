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


#include <QApplication>
#include <QMap>

#include "DevId.h"
#include "DevBase.h"

const QMap<int,QString>& DevId::buttons() const
{
    static QMap<int,QString> emptyButtons;
    if( drv ) return drv->buttons( *this );
    return emptyButtons;
}

void DevId::open() const
{
    if( drv ) qApp->postEvent( drv, new EventDevOpen(*this) );
}

void DevId::render( QImage img ) const
{
    if( drv ) qApp->postEvent( drv, new EventDevRender(*this,img) );
}

void DevId::close() const
{
    if( drv ) qApp->postEvent( drv, new EventDevClose(*this) );
}

QSize DevId::size() const
{
    if( drv ) return drv->size( *this );
    return QSize(320,240);
}

bool DevId::monochrome() const
{
    if( drv ) return drv->monochrome( *this );
    return false;
}

QDataStream & operator<< (QDataStream& stream, const DevId& id)
{
    stream  << id.name ;
    return stream;
}

QDataStream & operator>> (QDataStream& stream, DevId& id)
{
    id.drv = NULL;
    id.d = NULL;
    stream  >> id.name ;
    return stream;
}
