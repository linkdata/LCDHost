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

#ifndef NETWORKENGINE_WIN_H
#define NETWORKENGINE_WIN_H

#include <QString>
#include "NetworkEngine.h"

class NetworkEngine_NDIS;
class NetworkEngine_TDI;

class NetworkEngine_win : public NetworkEngine
{
public:
    NetworkEngine_win();
    ~NetworkEngine_win();

    void sample( lh_netdata *data, QString adapter = QString() );
    QStringList list() const;

private:
    NetworkEngine_NDIS *ndis_;
    NetworkEngine_TDI *tdi_;
};

#endif // NETWORKENGINE_WIN_H
