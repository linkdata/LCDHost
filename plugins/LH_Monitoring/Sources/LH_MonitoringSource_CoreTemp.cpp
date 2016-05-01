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

#include "LH_MonitoringSource_CoreTemp.h"
#include "LH_MonitoringTypes_CoreTemp.h"

LH_MonitoringSource_CoreTemp::LH_MonitoringSource_CoreTemp(LH_QtObject *parent): LH_MonitoringSource(parent, "CoreTemp")
{
    temp_ = SensorDefinition(QLatin1Literal("\260?"));
    freq_ = SensorDefinition(QLatin1Literal("MHz"));
    toTj_ = SensorDefinition(QLatin1Literal("\260? to TjMax"));
    perc_ = SensorDefinition(QLatin1Literal("%"), 0, 100);

}

bool LH_MonitoringSource_CoreTemp::doUpdate()
{
#ifndef Q_OS_WIN
    return false;
#else
    const char* mapname  = "CoreTempMappingObject";
    bool resultVal = false;

    HANDLE filemap = OpenFileMappingA(FILE_MAP_READ, FALSE, mapname);
    setDataAvailable(filemap != NULL);
    if(dataAvailable())
    {
        CTMemory* ctmemory = (CTMemory*)MapViewOfFile(filemap, FILE_MAP_READ, 0, 0, sizeof(CTMemory));

        if (ctmemory) {
            if( !((ctmemory->uiCPUCnt==0) && (ctmemory->uiCoreCnt==0)) )
            {
                resultVal = true;
                temp_.units = (ctmemory->ucFahrenheit? QLatin1Literal("\260F") : QLatin1Literal("\260C"));
                toTj_.units = temp_.units +  " to TjMax";

                updateFromArray("Core Temperature", "Core", ctmemory->uiCoreCnt, ctmemory->fTemp  , (ctmemory->ucDeltaToTjMax? toTj_ : temp_));
                updateFromArray("Core Load"       , "Core", ctmemory->uiCoreCnt, ctmemory->uiLoad , perc_);
                updateFromArray("CPU TjMax"       , "CPU" , ctmemory->uiCPUCnt , ctmemory->uiTjMax, temp_);

                updateValue("CPU Count","","CPU Count",ctmemory->uiCPUCnt);
                updateValue("Core Count","","Core Count",ctmemory->uiCoreCnt);
                updateValue("Voltage Identification","","Voltage Identification",ctmemory->fVID);
                updateValue("CPU Speed","","CPU Speed",ctmemory->fCPUSpeed, freq_);
                updateValue("FSB Speed","","FSB Speed",ctmemory->fFSBSpeed, freq_);
                updateValue("Multiplier","","Multiplier",ctmemory->fMultiplier);
                updateValue("CPU Name","","CPU Name",ctmemory->sCPUName);
            }
            UnmapViewOfFile(ctmemory);
        }
        CloseHandle(filemap);
    }
    return resultVal;
#endif
}

void LH_MonitoringSource_CoreTemp::updateFromArray(QString type, QString name, int count, const uint *valueSet, SensorDefinition def)
{
    if (count == 0)
        return;
    for(int i = 0; i<count; i++)
        updateValue(type, "", QString("%1 #%2").arg(name).arg(i+1), valueSet[i], def);
    updateAggregates(type, "");
}

void LH_MonitoringSource_CoreTemp::updateFromArray(QString type, QString name, int count, const float *valueSet, SensorDefinition def)
{
    if (count == 0)
        return;
    for(int i = 0; i<count; i++)
        updateValue(type, "", QString("%1 #%2").arg(name).arg(i+1), valueSet[i], def);
    updateAggregates(type, "");
}
