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


#include "DevBase.h"
#include "DevId.h"

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
#include "DevLgLcd.h"
#endif

QList<DevBase*> DevBase::drivers;

QList<DevId> DevBase::deviceList()
{
    QList<DevId> list;
    foreach( DevBase* drv, DevBase::drivers )
    {
        list.append( drv->list() );
    }
    return list;
}

void DevBase::init()
{
    drivers.clear();

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    DevBase *drv;
    drv = new DevLgLcd();
    if( drv )
    {
        drivers.append( drv );
        drv->start();
    }
#endif

    return;
}

void DevBase::term()
{
    foreach( DevBase *drv, drivers )
    {
        drv->quit();
        if( !drv->wait(1000) ) drv->terminate();
        delete drv;
    }

    drivers.clear();
    return;
}

void DevBase::setImage( QImage )
{
    return;
}
