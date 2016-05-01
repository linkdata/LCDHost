/**
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

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

#ifndef LH_QTPLUGIN_WEBKIT_H
#define LH_QTPLUGIN_WEBKIT_H

#include <QProcess>
#include <QTime>
#include "LH_QtPlugin.h"

class LH_QtPlugin_WebKit : public LH_QtPlugin
{
    QTime last_start_;

public:
    LH_QtPlugin_WebKit() : LH_QtPlugin() {}

    virtual void term();

    bool startServer();
    bool sendQuit();
};

#endif // LH_QTPLUGIN_WEBKIT_H
