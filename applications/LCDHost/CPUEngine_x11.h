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

#ifndef CPUENGINE_X11_H
#define CPUENGINE_X11_H

#include <stdio.h>
#include <QDateTime>
#include "CPUEngine.h"

class CPUEngine_x11 : public CPUEngine
{
    FILE* procstat_;
    int readProcStat(int count = 0, lh_cpudata* data = 0);
public:
    CPUEngine_x11();
    ~CPUEngine_x11();
    void sample( int, lh_cpudata * );
    qreal selfusage();
};

#endif // CPUENGINE_X11_H
