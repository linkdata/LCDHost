/**
  \file     LH_TextCoreTemp.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009 Johan Lindh

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.


  */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <QFont>
#include <QFontMetrics>
#include <QTime>

#include "LH_Text.h"
#include "../../LH_Qt_QStringList.h"

typedef struct core_temp_shared_data
{
    unsigned int	uiLoad[256];
    unsigned int	uiTjMax[128];
    unsigned int	uiCoreCnt;
    unsigned int	uiCPUCnt;
    float		fTemp[256];
    float		fVID;
    float		fCPUSpeed;
    float		fFSBSpeed;
    float		fMultipier;
    char		sCPUName[100];
    unsigned char	ucFahrenheit;
    unsigned char	ucDeltaToTjMax;
} CORE_TEMP_SHARED_DATA, *PCORE_TEMP_SHARED_DATA, **PPCORE_TEMP_SHARED_DATA;

class LH_TextCoreTemp : public LH_Text
{
    CORE_TEMP_SHARED_DATA ctsd_;
    LH_Qt_QStringList *setup_selector_;

    /**
      Check to see if CoreTemp is running, and if so, grab
      the latest data. Set the CPU count to zero if CoreTemp
      isn't running, to use as a marker. Return true if we
      should do refreshDisplay() next.
      */
    bool refreshData()
    {
        HANDLE hData;
        hData = OpenFileMappingA(FILE_MAP_READ, FALSE, "CoreTempMappingObject");
        if( hData != NULL )
        {
            CORE_TEMP_SHARED_DATA *ptr = (CORE_TEMP_SHARED_DATA *) MapViewOfFile(
                    hData, FILE_MAP_READ, 0, 0, sizeof(CORE_TEMP_SHARED_DATA));
            if( ptr != NULL )
            {
                memcpy( &ctsd_, ptr, sizeof(ctsd_) );
                UnmapViewOfFile(ptr);
            }
            CloseHandle(hData);
            return true;
        }
        if( ctsd_.uiCPUCnt == 0 )
        {
            // CoreTemp wasn't running before, and it still
            // isn't running, so no point in doing refreshDisplay()
            return false;
        }
        ctsd_.uiCPUCnt = 0;
        return true;
    }

    /**
      Using the current data and settings, update our text
      and return if it was changed (so the caller can
      make a refresh request when needed).
      */
    bool refreshDisplay()
    {
        if( ctsd_.uiCPUCnt > 0 && ctsd_.uiCoreCnt > 0 )
        {
            switch( setup_selector_->value() )
            {
            case 0: // Average core temp
                /* fall through */
            case 1: // Highest core temp
                if( ctsd_.uiCoreCnt )
                {
                    float total, avg, high;
                    total = avg = high = 0.0;
                    for( unsigned int i = 0; i < ctsd_.uiCoreCnt; i ++ )
                    {
                        float temp = ctsd_.fTemp[i];
                        if( temp < 1 ) temp = 0;
                        if( temp > 999 ) temp = 0;
                        if( temp > high ) high = temp;
                        total += temp;
                    }
                    avg = total / (float) ctsd_.uiCoreCnt;
                    return setText(
                            QString("%1\260%2")
                            .arg( (setup_selector_->value() == 1) ? high : avg, 0, 'f', 0 )
                            .arg( ctsd_.ucFahrenheit ? 'F' : 'C' )
                            );
                }
                break;
            case 2: // Core count
                return setText( QString::number(ctsd_.uiCoreCnt) );
            case 3: // CPU count
                return setText( QString::number(ctsd_.uiCPUCnt) );
            case 4: // Voltage ident
                return setText( QString::number(ctsd_.fVID,'f',3) );
            case 5: // CPU speed
                return setText( QString::number(ctsd_.fCPUSpeed,'f',1) );
            case 6: // FSB speed
                return setText( QString::number(ctsd_.fFSBSpeed,'f',1) );
            case 7: // Multiplier
                return setText( QString::number(ctsd_.fMultipier,'f',1) );
            case 8: // CPU name
                return setText( QString::fromLocal8Bit(ctsd_.sCPUName) );
            default:
                break;
            }
        }
        return setText("N/A");
    }

public:

    /**
      When making a class that integrates with another program,
      try to include a link to the program's homepage in the
      settings area.
      */
    LH_TextCoreTemp(const char *name) : LH_Text(name)
    {
        setup_text_->setName( tr("<a href=\"http://www.alcpu.com/CoreTemp/\">CoreTemp</a> data") );
        setup_text_->setFlag( LH_FLAG_READONLY, true );
        setText( "?%" );
        setup_selector_ = new LH_Qt_QStringList(this,tr("Show what"), QStringList() <<
                        tr("Average core temp") <<
                        tr("Highest core temp") <<
                        tr("Core count") <<
                        tr("CPU count") <<
                        tr("Voltage ident") <<
                        tr("CPU speed") <<
                        tr("FSB speed") <<
                        tr("Multiplier") <<
                        tr("CPU name"),
                        LH_FLAG_AUTORENDER|LH_FLAG_FIRST);
        setup_selector_->setHelp( "<p>Select which data item from CoreTemp to show.</p>" );
        memset( &ctsd_, 0, sizeof(ctsd_) );
        refreshData();
        return;
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "3rdParty/CoreTemp",
            "CoreTempText",
            "CoreTemp information (Text)",
            -1, -1,
            lh_instance_calltable_NULL
        };

        if( classInfo.width == -1 )
        {
            QFont font;
            QFontMetrics fm( font );
            classInfo.height = fm.height();
            classInfo.width = fm.width("100%");
        }

        return &classInfo;
    }

    /**
      Each second, grab data from CoreTemp, if they're available.
      If not, set CPU count to zero as a marker.
      */
    int notify(int n,void* p)
    {
        if( !n || n&LH_NOTE_SECOND )
        {
            if( refreshData() && refreshDisplay() ) callback(lh_cb_render,NULL);
        }

        return LH_Text::notify(n,p) | LH_NOTE_SECOND;
    }

    /**
      Generally, you'll want to refresh your data items in prerender()
      unless you know for a fact you *just* did it (using QTime::elapsed())
      or similar mechanism). If you don't, you won't get the instant
      user feedback when the user changes a setting or instantiates
      your class.

      Don't request a new render from here though, or you'll spin.
      */
    void prerender()
    {
        refreshDisplay();
        return;
    }
};

LH_PLUGIN_CLASS(LH_TextCoreTemp)

