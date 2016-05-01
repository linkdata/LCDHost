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

#include "LH_MonitoringSource_DriveStats.h"
//#include "LH_MonitoringTypes_DriveStats.h"

LH_MonitoringSource_DriveStats::LH_MonitoringSource_DriveStats(LH_QtObject *parent): LH_MonitoringSource(parent, "Drive Stats"), drives_()
{
    defPerc_ = SensorDefinition("%");

    defBytes_ = SensorDefinition("B");
    defBytes_.setAdaptiveUnits(1024, QStringList()<<"B"<<"kB"<<"MB"<<"GB"<<"TB");

    // Adaptive Unit behaviour will need to be coded seperately for:
    // o LH_MonitoringObject
    // o LH_Graph (due to it's special data access method.)
}

bool LH_MonitoringSource_DriveStats::doUpdate()
{
#ifndef Q_OS_WIN
    return false;
#else
    bool resultVal = true;
    setDataAvailable(true);

    drives_.update();

    foreach(DriveInfo drive, drives_.values())
    {
        updateValue("Bytes Read/Written", "Bytes Read", drive.driveLetter(), drive.BytesRead(), defBytes_);
        updateValue("Bytes Read/Written", "Bytes Written", drive.driveLetter(), drive.BytesWritten(), defBytes_);

        updateValue("Activity Counters", "Read Count", drive.driveLetter(), drive.ReadCount());
        updateValue("Activity Counters", "Write Count", drive.driveLetter(), drive.WriteCount());

        updateValue("Activity Percentages", "Read Time", drive.driveLetter(), drive.ReadTime(), defPerc_);
        updateValue("Activity Percentages", "Write Time", drive.driveLetter(), drive.WriteTime(), defPerc_);
        updateValue("Activity Percentages", "Idle Time", drive.driveLetter(), drive.IdleTime(), defPerc_);

        updateValue("Queue Depth", "", drive.driveLetter(), drive.QueueDepth());

        if (qlonglong total_space = drive.TotalSpace()) {
            updateValue("Disk Space", "Free Space Percent", drive.driveLetter(), (drive.FreeSpace() * 100) / total_space, defPerc_);
            updateValue("Disk Space", "Used Space Percent", drive.driveLetter(), (drive.UsedSpace() * 100) / total_space, defPerc_);
        } else {
            updateValue("Disk Space", "Free Space Percent", drive.driveLetter(), 0, defPerc_);
            updateValue("Disk Space", "Used Space Percent", drive.driveLetter(), 100, defPerc_);
        }

        updateValue("Disk Space", "Free Space", drive.driveLetter(), drive.FreeSpace(), defBytes_);
        updateValue("Disk Space", "Used Space", drive.driveLetter(), drive.UsedSpace(), defBytes_);
        updateValue("Disk Space", "Total Space", drive.driveLetter(), drive.TotalSpace(), defBytes_);


        updateValue("Last Update", "", drive.driveLetter(), drive.lastUpdate().toString());

        //Pie: "Disk Space"
    }

    return resultVal;
#endif
}
