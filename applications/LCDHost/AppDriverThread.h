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

#ifndef APPDRIVERTHREAD_H
#define APPDRIVERTHREAD_H

#include "AppLibraryThread.h"
class AppDriver;
class LibDriver;

class AppDriverThread : public AppLibraryThread
{
    Q_OBJECT

public:
    AppDriverThread( AppDriver *app_driver, QFileInfo fi, QObject *parent = NULL );

    LibLibrary *libCreate(AppId,QFileInfo fi);
    LibDriver *libDriver() const { return (LibDriver*)(libLibrary()); }
    AppDriver *appDriver() const { return (AppDriver*)(appLibrary()); }
};

#endif // APPDRIVERTHREAD_H
