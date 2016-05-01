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

#include "LCDHost.h"
#include "CPUEngine.h"
#include "CPUData.h"

CPUEngine::CPUEngine()
    : started_(QDateTime::currentDateTime())
    , count_(0)
{
}

CPUEngine::~CPUEngine()
{
}

void CPUEngine::sample( int n, lh_cpudata *p )
{
    if( p && n>0 ) memset( p, 0, sizeof(lh_cpudata) * n );
    return;
}

qreal CPUEngine::uptime() const
{
    return (qreal)(started_.msecsTo(QDateTime::currentDateTime())) / 1000.0;
}
