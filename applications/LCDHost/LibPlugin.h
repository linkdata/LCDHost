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


#if 0
#ifndef LIBPLUGIN_H
#define LIBPLUGIN_H

#include <QObject>
#include <QFileInfo>
#include <QBasicTimer>

#include "LibLibrary.h"
#include "lh_plugin.h"

// This class interfaces with the actual shared library
// It's created by the thread that runs the plugin

class AppPlugin;
class AppPluginThread;
class LibClass;
class LibInstance;

class LibPlugin : public LibLibrary
{
    Q_OBJECT

public:
    LibPlugin( AppId id, QFileInfo fi );
    ~LibPlugin();

    bool load();
    void postLoad();
    bool unload();
    lh_plugin_calltable& table() { return table_; }

    LibClass *getClassById( QString id ) const;
    LibInstance *getInstanceById( void* id ) const;

    const lh_class ** lh_class_list() { return table_.lh_class_list ? table_.lh_class_list() : NULL; }
    bool event( QEvent *event );

private:
    lh_plugin_calltable table_;
    QList<LibClass*> classList;

    void clear();
    void loadClassList();
};

#endif // LIBPLUGIN_H
#endif
