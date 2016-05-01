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

#ifndef NETWORKENGINE_H
#define NETWORKENGINE_H

#include <QStringList>

#include "lh_plugin.h"

class NetworkEngine
{
public:
    NetworkEngine() {}

    void sample( lh_netdata *data, QString filter = QString() ) { Q_UNUSED(data); Q_UNUSED(filter); }
    QStringList list() { return QStringList(); }
};

#ifdef Q_OS_WIN
# define NetworkEngine_xxx NetworkEngine_win
# include "NetworkEngine_win.h"
#endif

#ifdef Q_OS_MAC
# define NetworkEngine_xxx NetworkEngine_mac
# include "NetworkEngine_mac.h"
#endif

#ifdef Q_OS_LINUX
# define NetworkEngine_xxx NetworkEngine_x11
# include "NetworkEngine_x11.h"
#endif


#endif // NETWORKENGINE_H
