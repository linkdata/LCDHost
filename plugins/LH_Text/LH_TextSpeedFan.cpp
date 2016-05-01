/**
  \file     LH_TextSpeedFan.cpp
  @author   Andy Bridges <andy@bridgesuk.com>, Johan Lindh <johan@linkdata.se>
  \legalese
    This module is based on original work by Johan Lindh and uses code freely
    available from http://allthingsgeek.wordpress.com/ by "Noccy"

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

#include <QtDebug>
#include <QFont>
#include <QFontMetrics>
#include <QTime>
#include <QRegExp>

#include <stdio.h>
#include <windows.h>

#include "LH_TextSpeedFan.h"

LH_PLUGIN_CLASS(LH_TextSpeedFan)

// pragma pack is included here because the struct is a pascal Packed Record,
// meaning that fields aren't aligned on a 4-byte boundary. 4 bytes fit 2
// 2-byte records.
#pragma pack(1)

// This is the struct we're using to access the shared memory.
struct SFMemory {
        WORD version;
        WORD flags;
        int MemSize;
        int handle;
        WORD NumTemps;
        WORD NumFans;
        WORD NumVolts;
        signed int temps[32];
        signed int fans[32];
        signed int volts[32];
};

// JLI: Reset default packing alignment
#pragma pack()

lh_class *LH_TextSpeedFan::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "3rdParty/SpeedFan",
        "SpeedFanText",
        "SpeedFan Data",
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

LH_TextSpeedFan::LH_TextSpeedFan()
{
    setup_text_->setName( "<a href=\"http://www.almico.com/speedfan.php\">SpeedFan</a> display" );
    setup_text_->setFlag( LH_FLAG_READONLY, true );
    setup_template_ = new LH_Qt_QString(this,"Template Text",QString("Temp 1: %t1"));
    setup_template_->setOrder(-1);
    setup_instructions_ = new LH_Qt_QTextEdit(this,"~","Uses regex %(t|f|v)(n|[0-9]+).",LH_FLAG_LAST|LH_FLAG_READONLY);
    setText("N/A");
    return;
}

int LH_TextSpeedFan::notify(int n,void* p)
{
    if( !n || n&LH_NOTE_SECOND )
    {
        HANDLE filemap = OpenFileMappingA(FILE_MAP_READ, FALSE, "SFSharedMemory_ALM");
        if( filemap != NULL )
        {
            SFMemory* sfmemory = (SFMemory*)MapViewOfFile(filemap, FILE_MAP_READ, 0, 0, sizeof(SFMemory));
            if( sfmemory )
            {
                QString outText = setup_template_->value();
                QRegExp rx("%(t|f|v)(n|[0-9]+)");
                int pos = 0;
                while ((pos = rx.indexIn(outText, pos)) != -1) {
                    QString repVal;
                    if(rx.cap(1) == "t") {
                        if (rx.cap(2) == "n")
                            repVal = QString("%1").arg(sfmemory->NumTemps);
                        else
                            repVal = QString("%1\260C").arg(sfmemory->temps[rx.cap(2).toInt()]/100);
                    }
                    if(rx.cap(1) == "f") {
                        if (rx.cap(2) == "n")
                            repVal = QString("%1").arg(sfmemory->NumFans);
                        else
                            repVal = QString("%1").arg(sfmemory->fans[rx.cap(2).toInt()]);
                    }
                    if(rx.cap(1) == "v") {
                        if (rx.cap(2) == "n")
                            repVal = QString("%1").arg(sfmemory->NumVolts);
                        else
                            repVal = QString("%1V").arg(float(sfmemory->volts[rx.cap(2).toInt()])/100);
                    }
                    outText.replace(rx.cap(0),repVal);
                    pos += rx.matchedLength();
                }
                if( setText( outText ) )
                    callback(lh_cb_render,NULL); // only render if the text changed
            }
            else
            {
                if( setText( "Failed to open shared memory." ) )
                    callback(lh_cb_render,NULL); // only render if the text changed
            }
            // Close the handles we opened.
            CloseHandle(filemap);
        }
        else
            if( setText( "N/A" ) ) callback(lh_cb_render,NULL);
    }
    return LH_Text::notify(n,p) | LH_NOTE_SECOND;
};


