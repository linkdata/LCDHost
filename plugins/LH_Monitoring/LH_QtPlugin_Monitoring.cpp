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

#include "LH_QtPlugin_Monitoring.h"

LH_PLUGIN(LH_QtPlugin_Monitoring)

const char *LH_QtPlugin_Monitoring::userInit()
{
    // dataSources = new LH_MonitoringSources(this);
    dataSources()->add(new LH_MonitoringSource_Aida64(this));
    dataSources()->add(new LH_MonitoringSource_Afterburner(this));
    dataSources()->add(new LH_MonitoringSource_ATITrayTools(this));
    dataSources()->add(new LH_MonitoringSource_CoreTemp(this));
    dataSources()->add(new LH_MonitoringSource_Fraps(this));
    dataSources()->add(new LH_MonitoringSource_GPUZ(this));
    dataSources()->add(new LH_MonitoringSource_HWiNFO(this));
    dataSources()->add(new LH_MonitoringSource_HWMonitor(this));
    dataSources()->add(new LH_MonitoringSource_Logitech(this));
    dataSources()->add(new LH_MonitoringSource_RivaTuner(this));
    dataSources()->add(new LH_MonitoringSource_SpeedFan(this));
    // dataSources->userInit();

    return LH_QtMonitoringPlugin::userInit();
}



char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Monitoring</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" STRINGIZE(VERSION) "\nr" STRINGIZE(REVISION) "</ver>"
  "<versionurl>http://www.linkdata.se/lcdhost/version.php?arch=$ARCH</versionurl>"
  "<author>Andy \"Triscopic\" Bridges</author>"
  "<homepageurl><a href=\"http://www.codeleap.co.uk\">CodeLeap</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Connects to a variety of 3rd party applications and reports statistics such as Temperatures, Fan Speeds, Voltages, etc."
  "</shortdesc>"
  "<longdesc>"
"This plugin provides connections through to several 3rd Party applications, from which it can then extract information whenever they are running: "
"<br />"
"<table>"


"<tr>"
"<td colspan='3'> <hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/aida64.png\"/></td>"
"<td width=\"100%\"><a href=\"http://http://www.aida64.com\">Aida64</a></td>"
"<td align='right'>(Commercial)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>Aida64 monitors a very large number of system statistics as well as the computer's voltages, fan speeds and temperatures in computers equipped with monitoring chips.<br /><br />"
"<i>N.B. This plugin extracts the data from Aida64 via Shared Memory (make sure this option is enabled in Preferences > Hardware Monitoring > External Applications).</i><hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/atitray.png\"/></td>"
"<td width=\"100%\"><a href=\"http://www.alcpu.com/ATITrayTools/\">ATITrayTools</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>ATITrayTools monitors various sensors in ATI graphics cards.<hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/CoreTemp.png\"/></td>"
"<td width=\"100%\"><a href=\"http://www.alcpu.com/CoreTemp/\">CoreTemp</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>CoreTemp monitors a computer's temperature sensors in computers equipped with monitoring chips, as well as things like CPU and FSB speeds, etc.<hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/fraps.png\"/></td>"
"<td width=\"100%\"><a href=\"http://www.fraps.com/\">Fraps</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>The main use of Fraps is to acquire the current \"Frames Per Second\". It is primarily of use within games, but can be used in windows (check the setting to monitor WDM in Fraps).<hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/GPU-Z.png\"/></td>"
"<td width=\"100%\"><a href=\"http://www.techpowerup.com/gpuz/\">GPU-Z</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>GPU-Z monitors the various sensors in graphics cards.<hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/HWiNFO.png\"/></td>"
"<td width=\"100%\"><a href=\"http://www.hwinfo.com/\">HWiNFO</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>HWiNFO32 or HWiNFO64 monitors sensors on the motherboard, graphics cards, disk drives and more.<hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/HWMonitor.png\"/><img src=\":/images/apps/HwMonTray.png\"/></td>"
"<td width=\"100%\"><a href=\"http://www.hwinfo.com/\">HWMonitor</a> with <a href=\"http://blog.orbmu2k.de/tools/hardware-monitor-gadget-host\">HWMonTray</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>HWMonitor monitors graphics cards, motherboard sensors and disk drives. <i>However</i> it does not natively support sharing it's data. To solve this download HWMonTray aka \"HWMonitor GadgetHost\", and run HWMonTray which will then launch HWMonitor and make its data available to LCDHost.<hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/Logitech.png\"/></td>"
"<td width=\"100%\"><a href=\"http://forum.linkdata.se/lcdhost_plugins/windows_gadget_logitech_wireless_mouse_battery_monitoring_277.0.html/\">Logitech Mouse Battery Monitor</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>Not really an application, but an edited Wireless Mouse Battery monitoring gadget that exposes the battery level to LCDHost.<hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/MSIAfterburner.png\"/></td>"
"<td width=\"100%\"><a href=\"http://event.msi.com/vga/afterburner/\">MSI Afterburner</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>MSI Afterburner monitors a the various sensors in MSI graphics cards.<hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/RivaTuner.png\"/></td>"
"<td width=\"100%\"><a href=\"www.guru3d.com/index.php?page=rivatuner/\">RivaTuner</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>RivaTuner monitors a the various sensors in NVIDIA graphics cards.<hr/></td>"
"</tr>"

"<tr>"
"<td style=\"padding-right:2px\"><img src=\":/images/apps/speedfan.png\"/></td>"
"<td width=\"100%\"><a href=\"www.almico.com/speedfan.php\">SpeedFan</a></td>"
"<td align='right'>(Free)</td>"
"</tr>"
"<tr>"
"<td colspan='3'>SpeedFan monitors a computer's voltages, fan speeds and temperatures in computers equipped with monitoring chips.<hr/></td>"
"</tr>"


"</table>"
"</longdesc>"
"</lcdhostplugin>";

