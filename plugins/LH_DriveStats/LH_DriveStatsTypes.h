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

#ifndef LH_DRIVESTATSTYPES_H
#define LH_DRIVESTATSTYPES_H

#include <windows.h>
#include <winioctl.h>
#include <QString>
#include <QStringList>
#include <QMath.h>
#include <QHash>

struct DISK_PERFORMANCE_STATS {
    LARGE_INTEGER BytesRead;
    LARGE_INTEGER BytesWritten;
    LARGE_INTEGER ReadTime;
    LARGE_INTEGER WriteTime;
    LARGE_INTEGER IdleTime;
    DWORD         ReadCount;
    DWORD         WriteCount;
    DWORD         QueueDepth;
    DWORD         SplitCount;
    LARGE_INTEGER QueryTime;
    DWORD         StorageDeviceNumber;
    WCHAR         StorageManagerName[8];
  };

class DriveInfo
{
    DISK_PERFORMANCE_STATS diskPerformance;
    DISK_PERFORMANCE_STATS diskPerformanceOld;
    bool valid_;
    bool validOld_;
    bool validNew_;

    ULARGE_INTEGER TotalBytes_;
    ULARGE_INTEGER FreeBytes_;

    QString drive_;

    bool GetDrivePerformanceInfo()
    {
        HANDLE hDevice = CreateFileA(QString("\\\\.\\%1:").arg(drive_).toLatin1().data(),0,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

        if(hDevice == INVALID_HANDLE_VALUE)
            return false;

        DWORD NumBytesReturned;
        bool bResult = DeviceIoControl(hDevice, IOCTL_DISK_PERFORMANCE, NULL, 0, &diskPerformance, sizeof(diskPerformance), &NumBytesReturned, NULL);

        CloseHandle(hDevice);
        return bResult;
    }

    QString getLastErrorMessage()
    {
        DWORD dwError;
        wchar_t errBuf[256];

        dwError = GetLastError();
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)errBuf, sizeof(errBuf),NULL);

        return QString("Err Code: %1 - %2").arg(dwError).arg(QString::fromWCharArray(errBuf));
    }

public:
    DriveInfo(QString drive = "")
    {
        setDrive(drive);
    }

    bool setDrive(QString drive)
    {
        if(drive!="")
            drive = drive.at(0);
        if (drive_ != drive)
        {
            drive_ = drive;
            validOld_ = false;
            validNew_ = false;
            update();
            return true;
        }
        else
            return false;
    }


    bool valid()
    {
        return validOld_ && validNew_;
    }

    bool update()
    {
        diskPerformanceOld = diskPerformance;
        validOld_ = validNew_;
        validNew_ = (drive_==""? false : GetDrivePerformanceInfo());
        if(validNew_)
        {
            ULARGE_INTEGER BytesAvailable;
            BytesAvailable.QuadPart = 0L;
            TotalBytes_.QuadPart = 0L;
            FreeBytes_.QuadPart = 0L;

            GetDiskFreeSpaceExA(QString("%1:\\").arg(drive_).toLatin1().data(), &BytesAvailable, &TotalBytes_, &FreeBytes_);
        }
        return valid();
    }

    qlonglong BytesRead()
    {
        return(!valid()? 0 : diskPerformance.BytesRead.QuadPart - diskPerformanceOld.BytesRead.QuadPart);
    }

    qlonglong BytesWritten()
    {
        return(!valid()? 0 : diskPerformance.BytesWritten.QuadPart - diskPerformanceOld.BytesWritten.QuadPart);
    }

    qlonglong ReadCount()
    {
        return(!valid()? 0 : diskPerformance.ReadCount - diskPerformanceOld.ReadCount);
    }

    qlonglong WriteCount()
    {
        return(!valid()? 0 : diskPerformance.WriteCount - diskPerformanceOld.WriteCount);
    }

    double ReadTime()
    {
        //return(!valid()? 0 : ((DOUBLE)(diskPerformance.ReadTime.QuadPart - diskPerformanceOld.ReadTime.QuadPart))/10000000);

        qlonglong totaltime =
                (diskPerformance.ReadTime.QuadPart - diskPerformanceOld.ReadTime.QuadPart) +
                (diskPerformance.WriteTime.QuadPart - diskPerformanceOld.WriteTime.QuadPart) +
                (diskPerformance.IdleTime.QuadPart - diskPerformanceOld.IdleTime.QuadPart);

        return(!valid()? 0 : ((DOUBLE)(diskPerformance.ReadTime.QuadPart - diskPerformanceOld.ReadTime.QuadPart))/totaltime*100);
    }

    double WriteTime()
    {
        //return(!valid()? 0 : ((DOUBLE)(diskPerformance.WriteTime.QuadPart - diskPerformanceOld.WriteTime.QuadPart))/10000000);

        qlonglong totaltime =
                (diskPerformance.ReadTime.QuadPart - diskPerformanceOld.ReadTime.QuadPart) +
                (diskPerformance.WriteTime.QuadPart - diskPerformanceOld.WriteTime.QuadPart) +
                (diskPerformance.IdleTime.QuadPart - diskPerformanceOld.IdleTime.QuadPart);
        return(!valid()? 0 : ((DOUBLE)(diskPerformance.WriteTime.QuadPart - diskPerformanceOld.WriteTime.QuadPart))/totaltime*100);
    }

    double IdleTime()
    {
        //return(!valid()? 0 : ((DOUBLE)(diskPerformance.IdleTime.QuadPart - diskPerformanceOld.IdleTime.QuadPart))/10000000);

        qlonglong totaltime =
                (diskPerformance.ReadTime.QuadPart - diskPerformanceOld.ReadTime.QuadPart) +
                (diskPerformance.WriteTime.QuadPart - diskPerformanceOld.WriteTime.QuadPart) +
                (diskPerformance.IdleTime.QuadPart - diskPerformanceOld.IdleTime.QuadPart);
        return(!valid()? 0 : ((DOUBLE)(diskPerformance.IdleTime.QuadPart - diskPerformanceOld.IdleTime.QuadPart))/totaltime*100);
    }

    qlonglong QueueDepth()
    {
        return(!validNew_? 0 : diskPerformance.QueueDepth);
    }

    qlonglong FreeSpace()
    {
        return(!validNew_? 0 : FreeBytes_.QuadPart);
    }

    qlonglong TotalSpace()
    {
        return(!validNew_? 0 : TotalBytes_.QuadPart);
    }

    qlonglong UsedSpace()
    {
        return(!validNew_? 0 : TotalBytes_.QuadPart - FreeBytes_.QuadPart);
    }
};

#endif // LH_DRIVESTATSTYPES_H
