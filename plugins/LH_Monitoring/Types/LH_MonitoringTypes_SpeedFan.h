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

#ifndef LH_SPEEDFANTYPES_H
#define LH_SPEEDFANTYPES_H

#pragma pack(1)
struct SFMemory {
        quint16 version;
        quint16 flags;
        int MemSize;
        int handle;
        quint16 NumTemps;
        quint16 NumFans;
        quint16 NumVolts;
        signed int temps[32];
        signed int fans[32];
        signed int volts[32];
};
#pragma pack()

#endif // LH_SPEEDFANTYPES_H
