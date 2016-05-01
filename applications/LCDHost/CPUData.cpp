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

#include "CPUData.h"

qreal CPUData::load( int n, CPUData *since )
{
    qreal s, u, i;

    if( n<0 || n>=count_ ) return 0;

    if( since && since != this )
    {
        if( since->count_ != count_ ) return 0;
        s = core_[n].system - since->core_[n].system;
        u = core_[n].user - since->core_[n].user;
        i = core_[n].idle - since->core_[n].idle;
    }
    else
    {
        s = core_[n].system;
        u = core_[n].user;
        i = core_[n].idle;
    }

    if( s+u > 0 ) return 1.0 - (i / (s+u));
    return 0;
}
