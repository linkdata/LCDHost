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


#include <stdlib.h>
#include <string.h>

#include <QDebug>

#include "NetworkEngine_win.h"
#include "NetworkEngine_NDIS.h"
#include "NetworkEngine_TDI.h"

NetworkEngine_win::NetworkEngine_win()
{
    tdi_ = NULL;
    ndis_ = NULL;

    tdi_ = new NetworkEngine_TDI;
    if( !tdi_->working() )
    {
        delete tdi_;
        tdi_ = NULL;

        ndis_ = new NetworkEngine_NDIS;
        if( !ndis_->working() )
        {
            delete ndis_;
            ndis_ = NULL;
        }
    }

    return;
}

NetworkEngine_win::~NetworkEngine_win()
{
    if( ndis_ ) delete ndis_;
    if( tdi_ ) delete tdi_;
}

void NetworkEngine_win::sample( lh_netdata *data, QString adapter )
{
    if( data == NULL ) return;

    if( ndis_ ) ndis_->sample( data, adapter );
    else if( tdi_ ) tdi_->sample( data, adapter );

    return;
}

QStringList NetworkEngine_win::list() const
{
    if( ndis_ ) return ndis_->list();
    if( tdi_ ) return tdi_->list();
    return QStringList();
}
