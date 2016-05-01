/**
  \file     LH_QtNetwork.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

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

#include "LH_QtNetwork.h"

LH_QtNetwork::LH_QtNetwork( LH_QtObject *parent )
{
    parent_ = parent;
    setup_smoothing_ = new LH_Qt_QSlider(parent,"Smoothing",3,1,10,LH_FLAG_AUTORENDER);
    data_.clear();
    return;
}

LH_QtNetwork::~LH_QtNetwork()
{
    while( !data_.isEmpty() )
        delete data_.takeFirst();
    return;
}

int LH_QtNetwork::notify(int n, void *p)
{
    Q_UNUSED(p);
    Q_ASSERT( parent_ != NULL );

    if( n & LH_NOTE_NET )
    {
        while( data_.size() >= samples() ) delete data_.dequeue();
        lh_netdata *data = new lh_netdata;
        if( data )
        {
            memcpy( data, & state()->net_data, sizeof(lh_netdata) );
            while( data_.size() >= samples() ) delete data_.dequeue();
            data_.enqueue( data );
            if(!parent_->inherits(LH_QtPlugin::staticMetaObject.className()))
                parent_->requestRender();
        }
    }
    return LH_NOTE_NET;
}

qint64 LH_QtNetwork::inTotal() const {
    if (data_.isEmpty())
        return 0;
    return data_.last()->in;
}

qint64 LH_QtNetwork::outTotal() const {
    if (data_.isEmpty())
        return 0;
    return data_.last()->out;
}

qint64 LH_QtNetwork::inRate() const
{
    if( data_.size() < 2 ) return 0;
    if( data_.last()->device != data_.first()->device ) return 0;
    if( data_.last()->in < data_.first()->in ) return 0;
    qint64 timedelta = data_.last()->when - data_.first()->when;
    if( timedelta > 0 ) return (data_.last()->in - data_.first()->in) * Q_INT64_C(1000) / timedelta;
    return 0;
}

qint64 LH_QtNetwork::outRate() const
{
    if( data_.size() < 2 ) return 0;
    if( data_.last()->device != data_.first()->device ) return 0;
    if( data_.last()->out < data_.first()->out ) return 0;
    qint64 timedelta = data_.last()->when - data_.first()->when;
    if( timedelta > 0 ) return (data_.last()->out - data_.first()->out) * Q_INT64_C(1000) / timedelta;
    return 0;
}

int LH_QtNetwork::inPermille() const
{
    if( state()->net_max_in ) return inRate() * 1000 / state()->net_max_in;
    return 0;
}

int LH_QtNetwork::outPermille() const
{
    if( state()->net_max_out ) return outRate() * 1000 / state()->net_max_out;
    return 0;
}

int LH_QtNetwork::tpPermille() const
{
    qint64 div = state()->net_max_in + state()->net_max_out;
    if( div ) return (inRate() + outRate()) * 1000 / div;
    return 0;
}
