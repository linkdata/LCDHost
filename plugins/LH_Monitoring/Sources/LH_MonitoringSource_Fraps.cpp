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


#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "LH_MonitoringSource_Fraps.h"
#include "LH_MonitoringTypes_Fraps.h"

LH_MonitoringSource_Fraps::LH_MonitoringSource_Fraps(LH_QtObject *parent): LH_MonitoringSource(parent, "Fraps")
{
    fps_ = SensorDefinition("fps");
}

FRAPS_SHARED_DATA *(WINAPI *FrapsSharedData) ();
bool LH_MonitoringSource_Fraps::doUpdate()
{
#ifndef Q_OS_WIN
    return false;
#else
    HMODULE frapsDLL;
    FRAPS_SHARED_DATA *fsd;
#ifdef Q_PROCESSOR_X86_32
    frapsDLL = GetModuleHandleA("FRAPS32.DLL");
#else
    frapsDLL = GetModuleHandleA("FRAPS64.DLL");
#endif
    bool resultVal = false;

    setDataAvailable(frapsDLL != NULL);
    if(!dataAvailable()) // Not running
        pingFraps();
    else {
        FrapsSharedData = (FRAPS_SHARED_DATA *(WINAPI *)()) GetProcAddress(frapsDLL, "FrapsSharedData");

        if (!FrapsSharedData)
        {
            qWarning() << "This version of Fraps is incompatible with LCDHost: Fraps 1.9C or later is required!";
        } else {
            fsd = FrapsSharedData();
            updateValue("Current FPS","","",(int)fsd->currentFPS, fps_);
            updateValue("Game Name","","",fsd->gameName);
            //updateValue("Total Frames","","",(int)fsd->totalFrames);
            //updateValue("Time of Last Frame","","",(int)fsd->timeOfLastFrame);
            resultVal = true;
        }
    }
    return resultVal;
#endif
}

void LH_MonitoringSource_Fraps::pingFraps()
{
#ifdef Q_OS_WIN
    if( FindWindowA( "#32770", NULL ) != (HWND)NULL )
    {
        HWND hWndLH = FindWindowA( "QWidget" , "LCDHost" );
        if ( hWndLH != 0 )
        {
            HINSTANCE instance = (HINSTANCE)
#ifdef _WIN64
                    GetWindowLong(hWndLH, GWLP_HINSTANCE);
#else
                    GetWindowLong(hWndLH, GWL_HINSTANCE);
#endif

            HWND hWnd = CreateWindowA("QWidget", "LCDHost",
                                       WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                       (HWND) NULL,(HMENU) NULL, instance, (LPVOID) NULL);
            DestroyWindow( hWnd );
        }
    }
#endif
}

