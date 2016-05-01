/**
  \file     LH_VirtualLCD.cpp
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

#include <QFile>
#include <QDebug>

#include "LH_QtDevice.h"
#include "LH_VirtualLCD.h"
#include "LH_Qt_QImage.h"
#include "VirtualDevice.h"

LH_PLUGIN(LH_VirtualLCD)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Virtual LCD</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" "r" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Johan \"SirReal\" Lindh</author>"
  "<homepageurl><a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Virtual LCD driver"
  "</shortdesc>"
  "<longdesc>"
  "This driver provides two software emulated LCD's, one QVGA and one monochrome 160x43."
  "</longdesc>"
"</lcdhostplugin>";

class VirtualQVGA : public VirtualDevice
{
public:
    VirtualQVGA( LH_QtPlugin *drv ) : VirtualDevice( drv )
    {
        setDevid("320x240x32");
        setName("Virtual 320x240x32 device");
        setSize(320,240);
        setDepth(32);
        setAutoselect(false);
        arrive();
    }
};

class VirtualBW : public VirtualDevice
{
public:
    VirtualBW( LH_QtPlugin *drv ) : VirtualDevice( drv )
    {
        setDevid("160x43x1");
        setName("Virtual 160x43x1 device");
        setSize(160,43);
        setDepth(1);
        setAutoselect(false);
        arrive();
    }
};

const char *LH_VirtualLCD::userInit()
{
    new VirtualQVGA(this);
    new VirtualBW(this);
    return 0;
}
