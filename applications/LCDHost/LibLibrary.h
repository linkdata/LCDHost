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

#ifndef LIBLIBRARY_H
#define LIBLIBRARY_H

#include <QLibrary>
#include <QList>

#include "lh_plugin.h"
#include "AppId.h"
#include "LibObject.h"

class LibLibrary : public LibObject
{
    Q_OBJECT

    QLibrary *lib_;
    const lh_object_calltable *objtable_;
    lh_plugin_calltable table_;

public:
    LibLibrary( QLibrary *lib, AppId id );
    ~LibLibrary();

    virtual const lh_object_calltable *lock() { return objtable_; }
    virtual void unlock() {}

    virtual QString init();
    void loadComplete();
    virtual int notify(int, void *);
    virtual void term();

    bool event( QEvent * );

    LibObject *childFromId(AppId id);
    LibObject *childFromObj(const void *obj);
};

#endif // LIBLIBRARY_H
