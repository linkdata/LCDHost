/**
  \file     LH_QtPlugin_Decor.h
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

#ifndef LH_QTPLUGIN_DECOR_H
#define LH_QTPLUGIN_DECOR_H

#include "LH_QtPlugin.h"

class LH_QtPlugin_Decor : public LH_QtPlugin
{
public:
    const char * lh_name(void) { return "Decor"; }
    const char * lh_shortdesc(void) { return "Static decorations."; }
    const char * lh_author(void) { return "Johan \"SirReal\" Lindh"; }
    const char * lh_homepage(void) { return "<a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a>"; }
    const char * lh_longdesc(void) {  return "Backgrounds, rectangles, lines, circles."; }
};

#endif // LH_QTPLUGIN_DECOR_H
