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

#include "LH_MonitoringSource_SpeedFan.h"
#include "LH_MonitoringTypes_SpeedFan.h"

#ifdef Q_OS_WIN
# include <windows.h>
#endif

LH_MonitoringSource_SpeedFan::LH_MonitoringSource_SpeedFan(LH_QtObject *parent): LH_MonitoringSource(parent, "SpeedFan") {
    temp_ = SensorDefinition( QLatin1Literal("\260C"), -128.0 );
    fans_ = SensorDefinition( "rpm", 0 );
    volt_ = SensorDefinition( "V" );
}


bool LH_MonitoringSource_SpeedFan::doUpdate()
{
#ifndef Q_OS_WIN
    return false;
#else
    bool resultVal = false;

    HANDLE filemap = OpenFileMappingA(FILE_MAP_READ, FALSE, "SFSharedMemory_ALM");

    setDataAvailable(filemap != NULL);
    if(dataAvailable())
    {
        if (SFMemory* sfmemory = (SFMemory*)MapViewOfFile(filemap, FILE_MAP_READ, 0, 0, sizeof(SFMemory)))
        {
            if(sfmemory->MemSize!=0)
            {
                updateFromArray("Temperature", sfmemory->NumTemps, sfmemory->temps, 0.01, temp_);
                updateFromArray("Fan",         sfmemory->NumFans,  sfmemory->fans,  1.00, fans_);
                updateFromArray("Voltage",     sfmemory->NumVolts, sfmemory->volts, 0.01, volt_);
                resultVal = true;
            }
            UnmapViewOfFile(sfmemory);
        }
        CloseHandle(filemap);
    }
    return resultVal;
#endif
}

void LH_MonitoringSource_SpeedFan::updateFromArray(QString type, int count, const int *valueSet, qreal modifier, SensorDefinition def)
{
    if (count == 0)
        return;
    for(int i = 0; i<count; i++)
        updateValue(type, "", QString("%1 #%2").arg(type).arg(i+1), (modifier * (qreal)valueSet[i]), def);
    updateAggregates(type, "");
}

void LH_MonitoringSource_SpeedFan::updateFromArray(QString type, int count, const uint *valueSet, qreal modifier, SensorDefinition def)
{
    if (count == 0)
        return;
    for(int i = 0; i<count; i++)
        updateValue(type, "", QString("%1 #%2").arg(type).arg(i+1), (modifier * (qreal)valueSet[i]), def);
    updateAggregates(type, "");
}

void LH_MonitoringSource_SpeedFan::updateFromArray(QString type, int count, const qreal *valueSet, qreal modifier, SensorDefinition def)
{
    if (count == 0)
        return;
    for(int i = 0; i<count; i++)
        updateValue(type, "", QString("%1 #%2").arg(type).arg(i+1), (modifier * (qreal)valueSet[i]), def);
    updateAggregates(type, "");
}
