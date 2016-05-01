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

#ifndef LIBDRIVER_H
#define LIBDRIVER_H

#if 0

#include "LibLibrary.h"
#include "lh_driver.h"

class LibDriver : public LibLibrary
{
    Q_OBJECT

public:
    LibDriver( AppId id,  QFileInfo fi );
    ~LibDriver() {}

    lh_driver_calltable& table() { return table_; }
    void callback( const void *obj, lh_callbackcode code, void *param );

    bool load();
    bool unload();

    virtual int notify(int,void*);

    bool event( QEvent *event );

private:
    int notify_;
    lh_driver_calltable table_;
};

#endif

#endif // LIBDRIVER_H
