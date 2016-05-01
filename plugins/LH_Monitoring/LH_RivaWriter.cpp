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

#include "LH_RivaWriter.h"

#include <QDebug>
#include <QRegExp>
#include <algorithm>

LH_PLUGIN_CLASS(LH_RivaWriter)

LH_RivaWriter::LH_RivaWriter()
{
    createRivaMemory(0, 0xDEAD);
}

const char *LH_RivaWriter::userInit()
{
    if( const char *err = LH_QtInstance::userInit() ) return err;
    hide();
    return 0;
}

LH_RivaWriter::~LH_RivaWriter()
{
    destroyRivaMemory();
}

lh_class *LH_RivaWriter::classInfo()
{
#ifndef QT_NO_DEBUG
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Test",
        "RivaWriter",
        "RivaTuner DataWriter",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
#else
    return NULL;
#endif
}

int LH_RivaWriter::notify(int code,void* param)
{
    Q_UNUSED(code);
    Q_UNUSED(param);
    updateRivaMemory();
    return LH_NOTE_SECOND;
}


bool LH_RivaWriter::updateRivaMemory()
{
#ifdef Q_OS_WIN
    const char* mapname = "MAHMSharedMemory";
    bool resultVal = true;

    // Create file mapping
    HANDLE filemap = OpenFileMappingA(FILE_MAP_READ, FALSE, mapname);
    // Get pointer
    if(filemap != NULL)
    {
        MAHM_SHARED_MEMORY_HEADER* MAHMHeader = (MAHM_SHARED_MEMORY_HEADER*)MapViewOfFile(filemap, FILE_MAP_READ, 0, 0, 0);

        if (MAHMHeader) {

            destroyRivaMemory();
            if (createRivaMemory(MAHMHeader->dwNumEntries, MAHMHeader->dwSignature)) {
                for(int index = 0; index < (int)MAHMHeader->dwNumEntries; index ++)
                {
                    MAHM_SHARED_MEMORY_ENTRY* MAHMMemory = (MAHM_SHARED_MEMORY_ENTRY*)((LPBYTE)MAHMHeader + MAHMHeader->dwHeaderSize + index * MAHMHeader->dwEntrySize);

                    RTHM_SHARED_MEMORY_ENTRY* RTHMMemory = (RTHM_SHARED_MEMORY_ENTRY*)((LPBYTE)(RTHMHeader + 1) + index * RTHMHeader->dwEntrySize);

                    QString sensorName = QString(MAHMMemory->szSrcName);
                    QRegExp rx = QRegExp("([^0-9]*)([0-9]+)(.*)");
                    rx.setPatternSyntax(QRegExp::RegExp2);
                    sensorName = reverse(reverse(sensorName).replace(rx,"\\1\\3 :\\2#"));

                    memcpy( RTHMMemory->czSrc , sensorName.toLatin1().data(), 32);
                    memcpy( RTHMMemory->czDim , MAHMMemory->szSrcUnits, 16);
                    RTHMMemory->data = MAHMMemory->data;
                    RTHMMemory->offset = 0;
                    RTHMMemory->dataTransformed = MAHMMemory->data;
                }
            }
            else
                resultVal = false;


            UnmapViewOfFile(MAHMHeader);
        } else
            resultVal = false;
        CloseHandle(filemap);
    } else
        resultVal = false;


    return resultVal;
#else
    return false;
#endif
}

QString LH_RivaWriter::reverse(QString str)
{
    QByteArray ba = str.toLatin1();
    char *d = ba.data();
    std::reverse(d, d+str.length());
    return QString(d);
}

bool LH_RivaWriter::createRivaMemory(int entryCount, DWORD dwSignature)
{
#ifdef Q_OS_WIN
    const char* mapname_riva = "RTHMSharedMemory";

    DWORD sz = sizeof(RTHM_SHARED_MEMORY_HEADER) + entryCount * sizeof(RTHM_SHARED_MEMORY_ENTRY);
    filemap_riva = (HANDLE)CreateFileMappingA(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sz,mapname_riva);
    // Get pointer
    if(filemap_riva != NULL)
    {
        RTHMHeader = (RTHM_SHARED_MEMORY_HEADER*)MapViewOfFile(filemap_riva, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
        if (RTHMHeader) {
            memset(RTHMHeader, '\0', sizeof(RTHM_SHARED_MEMORY_HEADER));
            RTHMHeader->dwSignature = dwSignature;
            RTHMHeader->dwVersion = 1;
            RTHMHeader->dwNumEntries = entryCount;
            RTHMHeader->dwEntrySize = sizeof(RTHM_SHARED_MEMORY_ENTRY);
            return true;
        } else
            CloseHandle(filemap_riva);
    }
    return false;
#else
    Q_UNUSED(entryCount);
    Q_UNUSED(dwSignature);
    return false;
#endif
}

void LH_RivaWriter::destroyRivaMemory()
{
#ifdef Q_OS_WIN
    if (RTHMHeader) UnmapViewOfFile(RTHMHeader);
    if (filemap_riva) CloseHandle(filemap_riva);
#endif
}
