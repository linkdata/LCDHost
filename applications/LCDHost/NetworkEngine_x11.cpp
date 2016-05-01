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

#include <QDateTime>

#include "NetworkEngine_x11.h"

NetworkEngine_x11::NetworkEngine_x11()
{
    procnetdev_ = NULL;
    sample(NULL);
}

NetworkEngine_x11::~NetworkEngine_x11()
{
    if( procnetdev_ )
    {
        fclose( procnetdev_ );
        procnetdev_ = NULL;
    }
}

void NetworkEngine_x11::sample( lh_netdata *data, QString adapter )
{
    char buf[256];
    char name[64];
    int namelen;
    unsigned recv, xmit;

    list_.clear();
    if( procnetdev_ == NULL ) procnetdev_ = fopen("/proc/net/dev","r");
    else fseek(procnetdev_, 0, SEEK_SET);
    if( procnetdev_ == NULL ) return;

    if( data )
    {
        data->when = QDateTime::currentDateTime().toMSecsSinceEpoch();
        data->in = 0;
        data->out = 0;
    }

    while( !feof(procnetdev_) )
    {
        buf[0] = 0;
        buf[6] = 0;
        if( fgets(buf, sizeof(buf)-1, procnetdev_) == NULL) break;
        if( buf[6] == '|' ) continue;
        *name = 0; recv = xmit = 0;
        sscanf(buf," %64s %u %*u %*u %*u %*u %*u %*u %*u %u", name, &recv, &xmit );
        namelen = strlen(name);
        if( namelen>0 && name[namelen-1]==':' ) name[namelen-1] = 0;
        list_.append( QString(name) );
        if( data )
        {
            if( adapter.isEmpty() || adapter == name )
            {
                data->device = list_.size();
                data->in += recv;
                data->out += xmit;
            }
        }
    }

    if( data && adapter.isEmpty() )
        data->device = 0;

    return;
}

