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

#ifndef CPUENGINE_H
#define CPUENGINE_H

#include <QtGlobal>
#include <QDateTime>
#include "lh_plugin.h"

class CPUEngine
{
protected:
    QDateTime started_; // When the process started
    int count_; // Number of CPU cores

public:
    CPUEngine();
    ~CPUEngine();

    int count() const { return count_; }
    void sample( int, lh_cpudata * );
    qreal selfusage() { return 0.0; } // in seconds
    qreal uptime() const; // in seconds
};

#ifdef Q_OS_WIN
# define CPUEngine_xxx CPUEngine_win
# include "CPUEngine_win.h"
#endif

#ifdef Q_OS_MAC
# define CPUEngine_xxx CPUEngine_mac
# include "CPUEngine_mac.h"
#endif

#ifdef Q_OS_LINUX
# define CPUEngine_xxx CPUEngine_x11
# include "CPUEngine_x11.h"
#endif

#endif // CPUENGINE_H
