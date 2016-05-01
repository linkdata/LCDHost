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

#ifndef LH_GPUZTYPES_H
#define LH_GPUZTYPES_H

#ifdef Q_OS_WIN
# include <windows.h>
#endif

// pragma pack is included here because the struct is a pascal Packed Record,
// meaning that fields aren't aligned on a 4-byte boundary. 4 bytes fit 2
// 2-byte records.
#define MAX_RECORDS 128

#pragma pack(1)
// This is the struct we're using to access the shared memory.
struct GPUZ_RECORD
{
wchar_t key[256];
wchar_t value[256];
};

struct GPUZ_SENSOR_RECORD
{
wchar_t name[256];
wchar_t unit[8];
quint32 digits;
double value;
};

struct GPUZ_SH_MEM
{
quint32 version; // Version number, 1 for the struct here
volatile long busy;	 // Is data being accessed?
quint32 lastUpdate; // GetTickCount() of last update
GPUZ_RECORD data[MAX_RECORDS];
GPUZ_SENSOR_RECORD sensors[MAX_RECORDS];
};
#pragma pack()

#endif // LH_GPUZTYPES_H
