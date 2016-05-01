/**
  \file     LH_QtPlugin_Bar.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009 Johan Lindh

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

#ifndef LH_QTPLUGIN_BAR_H
#define LH_QTPLUGIN_BAR_H

#include "LH_QtPlugin.h"

class LH_QtPlugin_Bar : public LH_QtPlugin
{
    Q_OBJECT

public:
    const char * lh_name() { return "Bar"; }
    const char * lh_shortdesc() { return "Bar and histogram graphs"; }
    const char * lh_author() { return "Johan \"SirReal\" Lindh"; }
    const char * lh_homepage() { return "<a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a>"; }
    const char * lh_longdesc()
    {
        return  "<p>Bar and histogram displays, for example CPU, memory and network load displayed as bars.</p>"
                "<p>The source code for this plugin is included with the distribution.</p>";
    }

    const char *lh_load();
    void lh_unload();
};

#endif // LH_QTPLUGIN_BAR_H
