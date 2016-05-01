/**
  \file     LH_Fraps.cpp
  @author   Andy Bridges <andy@bridgesuk.com>
  \legalese
    This module is based on original work by Johan Lindh and uses code freely
    available from the Fraps v1.9D release notes (hwi.ath.cx/ut/Fraps/readme.htm)

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

#include <QFont>
#include <QFontMetrics>
#include <QTime>

#include <stdio.h>
#include <windows.h>

#include "LH_Text.h"

struct FRAPS_SHARED_DATA {
   DWORD sizeOfStruct;
   DWORD currentFPS;
   DWORD totalFrames;
   DWORD timeOfLastFrame;
   char gameName[32];
};

FRAPS_SHARED_DATA *(WINAPI *FrapsSharedData) ();

class LH_TextFraps : public LH_Text
{
public:
    LH_TextFraps() : LH_Text()
    {
        setup_text_->setName( "<a href=\"http://www.fraps.com/\">Fraps</a> FPS count" );
        setup_text_->setFlag( LH_FLAG_READONLY, true );
        setText("N/A");
        return;
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "3rdParty/Fraps",
            "FrapsFPSText",
            "Fraps FPS",
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
      In this class, we set the text to render in notify() instead of prerender().
      We do this because rendering can be done a lot more often than we ask for it,
      and we just need to update the load text when the LH_NOTE_SECOND notification
      fires. We use LH_NOTE_SECOND because the average load value lh_systemstate->cpu_load
      updates once a second. We'll also do the work if the value for 'n' is zero, since
      that's the initial call to notify right after the instance has been created.
      */
    int notify(int n,void* p)
    {
        if( !n || n&LH_NOTE_SECOND )
        {
            HMODULE frapsDLL;
            FRAPS_SHARED_DATA *fsd;
            frapsDLL = GetModuleHandleA("FRAPS32.DLL");
            if (!frapsDLL) {
                if( setText("N/A") ) callback(lh_cb_render,NULL);
            } else {
                FrapsSharedData = (typeof(FrapsSharedData)) GetProcAddress(frapsDLL, "FrapsSharedData");
                if (!FrapsSharedData) {
                    if( setText("Needs Fraps 1.9C or later!") ) callback(lh_cb_render,NULL);
                } else {
                    if( setText( "Fraps is running & is the right version." ) ) callback(lh_cb_render,NULL);
                    fsd = FrapsSharedData();
                    if( setText(QString::number(fsd->currentFPS) ) ) callback(lh_cb_render,NULL);
                }
            }
        }
        return LH_Text::notify(n,p) | LH_NOTE_SECOND;
    }

};

LH_PLUGIN_CLASS(LH_TextFraps)
