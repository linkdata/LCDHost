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

#ifndef CPUENGINE_WIN_H
#define CPUENGINE_WIN_H

#include "CPUEngine.h"

class CPUEngine_win : public CPUEngine
{
    void* m_sppi_ptr;
    size_t m_sppi_cb;
protected:
    int getSystemTimes();
    int getSPPI();
public:
    CPUEngine_win();
    ~CPUEngine_win();
    void sample(int n, lh_cpudata* p);
    qreal selfusage();
};

#endif // CPUENGINE_WIN_H
