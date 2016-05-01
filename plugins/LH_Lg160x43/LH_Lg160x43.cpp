/**
  \file     LH_Lg160x43.cpp
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

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "LH_Lg160x43.h"
#include "Lg160x43Device.h"
#include "LH_HidDevice.h"

LH_PLUGIN(LH_Lg160x43)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Lg160x43</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" "r" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Johan \"SirReal\" Lindh</author>"
  "<homepageurl><a href=\"http://www.linkdata.se/software/lcdhost\">Link Data Stockholm</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Logitech 160x43 LCD via HID"
  "</shortdesc>"
  "<longdesc>"
    "HID-level driver for Logitech 160x43 LCD displays, such as G13 or G15.<br/>"
    "Note that to use this driver you may need to uninstall existing drivers for these devices."
  "</longdesc>"
"</lcdhostplugin>";

const char *LH_Lg160x43::userInit()
{
#ifdef Q_OS_WIN
    // make sure neither LCDMon.exe nor LCORE.EXE is running on Windows
    if( FindWindowA( "Logitech LCD Monitor Window", "LCDMon" ) ||
        FindWindowA( "QWidget", "LCore" ) )
        return "Logitech drivers are loaded";
#endif
    LH_HidDevice::subscribe(this, SLOT(onlineChanged(LH_HidDevice*,bool)));
    return 0;
}

void LH_Lg160x43::onlineChanged(LH_HidDevice *hd, bool b)
{
    if(b && hd->vendor_id() == 0x046d)
    {
        switch(hd->product_id())
        {
        case 0xC222: /* G15 */
        case 0x0A07: /* Z10 */
        case 0xC227: /* G15v2 */
        case 0xC21C: /* G13 */
        case 0xC22D: /* G510 without audio */
        case 0xC22E: /* G510 with audio */
            new Lg160x43Device(hd, 3, this);
            break;
        }
    }
}
