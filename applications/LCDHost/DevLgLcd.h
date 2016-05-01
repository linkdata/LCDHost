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

#ifndef DEVLGLCD_H
#define DEVLGLCD_H

#include "DevBase.h"

class LgLcdData;
class LgLcdDevice;

class DevLgLcd : public DevBase
{
    Q_OBJECT

public:
    DevLgLcd();
    virtual ~DevLgLcd();

    void run();
    bool event( QEvent * e );
    void timerEvent( QTimerEvent * event );
    void connect();
    void disconnect();
    void close( LgLcdDevice *idd );

    QList<DevId> list();
    QSize size( DevId );
    bool monochrome( DevId );
    const QMap<int,QString>& buttons( DevId );
    void open( DevId );
    void render( DevId, QImage );
    void close( DevId );

    static bool err( int result, const char *filename, int line );

private:
    int timerId;
    LgLcdData *d;
};

#endif // DEVLGLCD_H
