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

#include "LH_MonitoringSource_GPUZ.h"
#include "LH_MonitoringTypes_GPUZ.h"

LH_MonitoringSource_GPUZ::LH_MonitoringSource_GPUZ(LH_QtObject *parent): LH_MonitoringSource(parent, "GPU-Z") {}


bool LH_MonitoringSource_GPUZ::doUpdate()
{
#ifndef Q_OS_WIN
    return false;
#else
    bool resultVal = false;
    const char* mapname  = "GPUZShMem";

    HANDLE filemap = OpenFileMappingA(FILE_MAP_READ, FALSE, mapname);
    setDataAvailable(filemap != NULL);
    if(dataAvailable())
    {
        GPUZ_SH_MEM* gpuzMemory = (GPUZ_SH_MEM*)MapViewOfFile(filemap, FILE_MAP_READ, 0, 0, sizeof(GPUZ_SH_MEM));

        if (gpuzMemory) {
            if(gpuzMemory->version!=0)
            {
                resultVal = true;
                for(int i=0; i<MAX_RECORDS; i++)
                {
                    GPUZ_SENSOR_RECORD sensor = gpuzMemory->sensors[i];
                    QString name = QString::fromWCharArray(sensor.name);
                    if(name=="")
                        break;
                    else
                    {
                        QString units = QString::fromWCharArray(sensor.unit);
                        if(units=="%%") units = "%";
                        qreal value = sensor.value;

                        SensorDefinition def = SensorDefinition(units);
                        if(units=="%%")
                        {
                            minmax _tmp = { {true, 0}, {true, 100} };
                            def.limits = _tmp;
                        }

                        updateValue(name, "", "", value, def);
                    }
                }
            }
            UnmapViewOfFile(gpuzMemory);
        }
        CloseHandle(filemap);
    }

    return resultVal;
#endif
}
