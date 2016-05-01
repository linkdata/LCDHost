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

#ifndef NETWORKENGINE_TDI_H
#define NETWORKENGINE_TDI_H

#include <QtGlobal>
#include <QList>
#include <QStringList>
#include <QDateTime>

#ifdef Q_OS_WIN
# ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0501 // 0x0501 WinXP, 0x0601 WinVista
# endif
# include <windows.h>
#endif

#include "lh_plugin.h"

class TDI_Entity;

class NetworkEngine_TDI
{
public:
    NetworkEngine_TDI();
    ~NetworkEngine_TDI();

    QStringList list() const;
    void sample( lh_netdata *data, QString which = QString() );
    bool working() const;

private:
    HANDLE tcpdriver_;
    QList<TDI_Entity*> list_;
};

#endif // NETWORKENGINE_TDI_H
