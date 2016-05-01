/**
  \file     LH_VirtualLCD.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

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

#ifndef LH_VIRTUALLCD_H
#define LH_VIRTUALLCD_H

#include "LH_QtPlugin.h"

class LH_VirtualLCD : public LH_QtPlugin
{
    Q_OBJECT

public:
    LH_VirtualLCD() : LH_QtPlugin() {}

    const char *userInit();
};

#endif // LH_VIRTUALLCD_H
