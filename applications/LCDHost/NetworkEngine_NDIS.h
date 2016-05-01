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

#ifndef NETWORKENGINE_NDIS_H
#define NETWORKENGINE_NDIS_H

#include <QList>
#include <QStringList>

#include "lh_plugin.h"

class NDIS_Adapter;

class NetworkEngine_NDIS
{
    QList<NDIS_Adapter*> list_;

public:
    NetworkEngine_NDIS();
    ~NetworkEngine_NDIS();

    QStringList list() const;
    void sample( lh_netdata *data, QString which = QString() );
    bool working() const;
};

#endif // NETWORKENGINE_NDIS_H
