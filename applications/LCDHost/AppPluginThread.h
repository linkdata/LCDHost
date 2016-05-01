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

#ifndef APPPLUGINTHREAD_H
#define APPPLUGINTHREAD_H

#include <QDebug>
#include "AppLibraryThread.h"

class LibPlugin;
class AppPlugin;

// This class defines the thread type that drives a plugin
// It's created by a AppPlugin, and it creates the LibPlugin

class AppPluginThread : public AppLibraryThread
{
    Q_OBJECT

public:
    AppPluginThread( AppPlugin *app_plugin, QFileInfo fi, QObject *parent = NULL );

    bool invalid() const { return invalid_; }

    virtual LibLibrary *libCreate( AppId id, QFileInfo fi );
    LibPlugin *libPlugin() const { return (LibPlugin*)(libLibrary()); }
    AppPlugin *appPlugin() const { return (AppPlugin*)(appLibrary()); }

private:
    bool invalid_;
};

#endif // APPPLUGINTHREAD_H
