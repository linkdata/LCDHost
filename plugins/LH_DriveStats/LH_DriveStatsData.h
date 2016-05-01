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

#ifndef LH_DRIVESTATSDATA_H
#define LH_DRIVESTATSDATA_H

#include "LH_MonitoringData.h"
#include "LH_MonitoringUI.h"

#include "LH_DriveStatsTypes.h"

class LH_DriveStatsData: public LH_MonitoringData
{
    DriveInfo driveInfo;
    void parseBytes(qlonglong bytes, float& value, QString& text, QString& units);

protected:
    bool getData(float& value, QString& text, QString& units);
    bool getUpperLimit(float &value);
    bool getAdaptiveUnitOptions(unitOptionsType &options);

public:
    LH_DriveStatsData( LH_QtObject *parent, LH_MonitoringUI *ui, monitoringDataMode dataMode = mdmNumbers, bool includeGroups = false);

    QString appName() { return "Drive Stats"; }
    monitoringDataType getType();
};

#endif // LH_DRIVESTATSDATA_H
