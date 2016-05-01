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

#ifndef LH_CORETEMPTYPES_H
#define LH_CORETEMPTYPES_H

#ifdef Q_OS_WIN
# include <windows.h>
#endif

struct CTMemory
{
        unsigned int	uiLoad[256];
        unsigned int	uiTjMax[128];
        unsigned int	uiCoreCnt;
        unsigned int	uiCPUCnt;
        float           fTemp[256];
        float           fVID;
        float           fCPUSpeed;
        float           fFSBSpeed;
        float           fMultiplier;
        char            sCPUName[100];
        unsigned char	ucFahrenheit;
        unsigned char	ucDeltaToTjMax;
};

#endif // LH_CORETEMPTYPES_H
