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

#include "LH_MonitoringSource_Afterburner.h"
#include "LH_MonitoringTypes_Afterburner.h"
#include <QRegExp>
#include <QSharedMemory>

LH_MonitoringSource_Afterburner::LH_MonitoringSource_Afterburner(LH_QtObject *parent): LH_MonitoringSource(parent, "MSI Afterburner") {}


bool LH_MonitoringSource_Afterburner::doUpdate()
{
#ifndef Q_OS_WIN
    return false;
#else
    bool resultVal = false;

    HANDLE filemap = OpenFileMappingA(FILE_MAP_READ, FALSE, "MAHMSharedMemory");
    setDataAvailable(filemap != NULL);
    if(dataAvailable())
    {
        MAHM_SHARED_MEMORY_HEADER* MAHMHeader = (MAHM_SHARED_MEMORY_HEADER*)MapViewOfFile(filemap, FILE_MAP_READ, 0, 0, 0);

        if (MAHMHeader)
        {
            if (MAHMHeader->dwSignature == 0xDEAD)
                qDebug() << "LH_MonitoringSource_RivaTuner: Shared memory has been terminated; try again later.";
            else
            {
                QRegExp rx = QRegExp("([^0-9]*)([0-9]+)(.*)");
                rx.setPatternSyntax(QRegExp::RegExp2);
                for (uint i=0; i<MAHMHeader->dwNumEntries; i++)
                {
                    MAHM_SHARED_MEMORY_ENTRY* MAHMMemory = (MAHM_SHARED_MEMORY_ENTRY*)((uchar*)MAHMHeader + MAHMHeader->dwHeaderSize + i * MAHMHeader->dwEntrySize);
                    QString sensorName = QString(MAHMMemory->szSrcName);
                    QString sensorGroup = (rx.indexIn(reverse(sensorName))>-1? reverse(rx.cap(3)) + reverse(rx.cap(1)) : sensorName);

                    SensorDefinition def = SensorDefinition( QString(MAHMMemory->szSrcUnits), MAHMMemory->minLimit, MAHMMemory->maxLimit );
                    updateValue(sensorGroup,"",sensorName,(qreal)MAHMMemory->data, def);
                }
                foreach(SensorType type, sensors_)
                    foreach(SensorGroup group, type.groups)
                        updateAggregates(type.name, group.name);
                resultVal = true;
            }
            UnmapViewOfFile(MAHMHeader);
        }
        CloseHandle(filemap);
    }

    return resultVal;
#endif
}
