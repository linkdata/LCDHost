/**
  \file     LH_LgLcdLegacyThread.h
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

#ifndef LH_LGLCDLEGACYTHREAD_H
#define LH_LGLCDLEGACYTHREAD_H

#include "LH_LgLcdThread.h"
#include <QImage>

class LH_LgLcdLegacyThread : public LH_LgLcdThread
{
    Q_OBJECT

    bool enumerate( int conn );
    bool render( int conn );

public:
    static lgLcdOpenContext bw_cxt;
    static lgLcdOpenContext qvga_cxt;

    explicit LH_LgLcdLegacyThread(QObject *parent = 0) : LH_LgLcdThread(parent) {}

    void run();

    bool hasBW() const { return bw_cxt.index != -1; }
    bool hasQVGA() const { return qvga_cxt.index != -1; }

#ifdef Q_OS_WIN
    static DWORD WINAPI LH_LogitechButtonCB(int device, DWORD dwButtons, const PVOID pContext);
#endif
#ifdef Q_OS_MAC
    static unsigned long LH_LogitechButtonCB(int device, unsigned long dwButtons, const void* pContext);
#endif
};

#endif // LH_LGLCDLEGACYTHREAD_H
