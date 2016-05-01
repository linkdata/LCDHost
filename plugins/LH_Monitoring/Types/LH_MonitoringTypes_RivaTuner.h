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

#ifndef LH_RIVATUNERTYPES_H
#define LH_RIVATUNERTYPES_H

#include <QtGlobal>
#ifdef Q_OS_WIN
# include <windows.h>
#endif

struct RTHM_SHARED_MEMORY_HEADER
{
    DWORD	dwSignature;
            //signature allows applications to verify status of shared memory

            //The signature can be set to:
            //'RTHM'	- hardware monitoring memory is initialized and contains
            //			valid data
            //0xDEAD	- hardware monitoring memory is marked for deallocation and
            //			no longer contain valid data
            //otherwise the memory is not initialized
    DWORD	dwVersion;
            //header version ((major<<16) + minor)
            //must be set to 0x00010001 or greater to use this structure
    DWORD	dwNumEntries;
            //number of subsequent RTHM_SHARED_MEMORY_ENTRY entries
    time_t	time;
            //last polling time
    DWORD	dwEntrySize;
            //size of entries in subsequent RTHM_SHARED_MEMORY_ENTRY entries array
};
#define	RTHM_SHARED_MEMORY_ENTRY_FLAG_SHOW_IN_OSD   0x00000001 //item is configured to display in On-Screen Display
#define	RTHM_SHARED_MEMORY_ENTRY_FLAG_SHOW_IN_LCD   0x00000002 //item is configured to display in Logitech keyboard LCD
#define	RTHM_SHARED_MEMORY_ENTRY_FLAG_SHOW_IN_TRAY  0x00000004 //item is configured to display in tray icon

struct RTHM_SHARED_MEMORY_ENTRY
{
    char	czSrc[32];
            //data source name (e.g. "Core temperature")
    char	czDim[16];
            //data source dimansion (e.g. QLatin1Literal("\260C"))
    float	data;
            //last polled data in raw format (e.g. 50\260C)
            //(this field can be set to FLT_MAX if data is not available)

            //take a note that the user can enable raw data transforming mode and
            //display different values on the graph (e.g. raw core VID values can
            //be translated to mapped core voltages).
            //In this case you may use dataTransformed field to get transformed value
    float	offset;
            //user specified data offset (e.g. +10\260C temerature compensation)

            //take a note that this value is not automatically added to transformed
            //data, it is up to you to add offset to it
    float	dataTransformed;
            //last polled data in transformed format
    DWORD	flags;
            //bitmask, containing combination of RTHM_SHARED_MEMORY_ENTRY_FLAG_... flags
};

#endif // LH_RIVATUNERTYPES_H
