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

#include "LH_MonitoringSource_RivaTuner.h"
#include "LH_MonitoringTypes_RivaTuner.h"
#include <QRegExp>
#include <QSharedMemory>

LH_MonitoringSource_RivaTuner::LH_MonitoringSource_RivaTuner(LH_QtObject *parent): LH_MonitoringSource(parent, "RivaTuner") {}

bool LH_MonitoringSource_RivaTuner::doUpdate()
{
#ifndef Q_OS_WIN
    return false;
#else
    bool resultVal = false;

    HANDLE filemap = OpenFileMappingA(FILE_MAP_READ, FALSE, "RTHMSharedMemory");
    setDataAvailable(filemap != NULL);
    if(dataAvailable())
    {
        RTHM_SHARED_MEMORY_HEADER* RTHMHeader = (RTHM_SHARED_MEMORY_HEADER*)MapViewOfFile(filemap, FILE_MAP_READ, 0, 0, 0);

        if (RTHMHeader)
        {
            if (RTHMHeader->dwSignature == 0xDEAD)
                qDebug() << "LH_MonitoringSource_RivaTuner: Shared memory has been terminated; try again later.";
            else
            {
                QRegExp rx = QRegExp("([^#0-9]*)([#0-9]+)(.*)");
                rx.setPatternSyntax(QRegExp::RegExp2);
                for (uint i=0; i<RTHMHeader->dwNumEntries; i++)
                {
                    RTHM_SHARED_MEMORY_ENTRY* RTHMMemory = (RTHM_SHARED_MEMORY_ENTRY*)((uchar*)(RTHMHeader + 1) + i * RTHMHeader->dwEntrySize);
                    QString sensorName = QString(RTHMMemory->czSrc);
                    QString sensorGroup = (rx.indexIn(reverse(sensorName))>-1? reverse(rx.cap(3)) + reverse(rx.cap(1)) : sensorName);
                    if(sensorGroup.startsWith(":")) sensorGroup.remove(0,1);
                    sensorGroup = sensorGroup.trimmed();

                    SensorDefinition def = SensorDefinition(QString(RTHMMemory->czDim));

                    updateValue(sensorGroup,"Standard Data",sensorName, QVariant((qreal)RTHMMemory->data), def);

                    if(RTHMMemory->dataTransformed != RTHMMemory->data)
                        updateValue(sensorGroup,"Transformed Data",sensorName, QVariant((qreal)RTHMMemory->dataTransformed), def);

                    if(RTHMMemory->offset != 0)
                    {
                        updateValue(sensorGroup,"Standard Data + Offset",sensorName, QVariant((qreal)RTHMMemory->data+(qreal)RTHMMemory->offset), def);
                        if(RTHMMemory->dataTransformed != RTHMMemory->data)
                            updateValue(sensorGroup,"Transformed Data + Offset",sensorName, QVariant((qreal)RTHMMemory->dataTransformed+(qreal)RTHMMemory->offset), def);
                    }
                }
                foreach(SensorType type, sensors_)
                    foreach(SensorGroup group, type.groups)
                        updateAggregates(type.name, group.name);
                resultVal = true;
            }
            UnmapViewOfFile(RTHMHeader);
        }
        CloseHandle(filemap);
    }

    return resultVal;
#endif
}
