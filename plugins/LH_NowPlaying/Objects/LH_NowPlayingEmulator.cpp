/**
  \file     LH_NowPlayingEmulator.cpp
  @author   Johan Lindh <johan@linkdata.se>
  @author   Andy Bridges <andy@bridgesuk.com>
  Copyright (c) 2010 Johan Lindh, Andy Bridges

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


  **/

#include "LH_NowPlayingEmulator.h"

#include <windows.h>
#include <QDebug>

#define MSN_MAGIC_NUMBER 0x547

LH_PLUGIN_CLASS(LH_NowPlayingEmulator)

lh_class *LH_NowPlayingEmulator::classInfo()
{
#ifndef QT_NO_DEBUG
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "3rdParty/Music",
        "NowPlayingEmulator",
        "Now Playing Emulator (Text)",
        -1, -1,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };
#if 0
    if( classInfo.width == -1 )
    {
        QFont font;
        QFontMetrics fm( font );
        classInfo.height = fm.height();
        classInfo.width = fm.width("100%");
    }
#endif
    return &classInfo;
#else
    return NULL;
#endif
}

LH_NowPlayingEmulator::LH_NowPlayingEmulator()
    : LH_Text()
    , setup_player_(0)
    , setup_enabled_(0)
    , setup_artist_(0)
    , setup_track_(0)
    , setup_album_(0)
{
}

const char *LH_NowPlayingEmulator::userInit()
{
    if (const char* msg = LH_Text::userInit()) return msg;

    setup_text_->setFlag(LH_FLAG_HIDDEN, true);
    setup_text_->setFlag(LH_FLAG_NOSAVE_DATA, true);
    setup_text_->setFlag(LH_FLAG_NOSAVE_LINK, true);
    setup_font_->setFlag(LH_FLAG_HIDDEN, true);
    setup_fontresize_->setFlag(LH_FLAG_HIDDEN, true);
    setup_pencolor_->setFlag(LH_FLAG_HIDDEN, true);
    setup_bgcolor_->setFlag(LH_FLAG_HIDDEN, true);
    setup_horizontal_->setFlag(LH_FLAG_HIDDEN, true);
    setup_vertical_->setFlag(LH_FLAG_HIDDEN, true);
    setup_scrollrate_->setFlag(LH_FLAG_HIDDEN, true);
    setup_scrollstep_->setFlag(LH_FLAG_HIDDEN, true);
    setText("Msn Compatible Music Player Emulator");

    setup_player_  = new LH_Qt_QString(this,"Player Name","TestPlayer");
    setup_enabled_ = new LH_Qt_bool(this,"Player Active",true);
    setup_artist_  = new LH_Qt_QString(this,"Artist Name","Artist");
    setup_track_   = new LH_Qt_QString(this,"Track Name","Track");
    setup_album_   = new LH_Qt_QString(this,"Album Name","Album");
    return 0;
}

LH_NowPlayingEmulator::~LH_NowPlayingEmulator()
{
    sendMessage(false);
}

int LH_NowPlayingEmulator::notify(int code, void *param)
{
    Q_UNUSED(param);
    if(code & LH_NOTE_SECOND )
        sendMessage(setup_enabled_->value());
    return LH_NOTE_SECOND;
}

void LH_NowPlayingEmulator::sendMessage(bool enabled)
{
    HWND hwnd = ::FindWindowW(L"MsnMsgrUIManager", NULL);
    qDebug() << "window: " << hwnd;
    if(hwnd!=0)
    {
        wchar_t wstr[512];
        QString msg = QString("%1\\0Music\\0%2\\0%1: {1} - {0}\\0%3\\0%4\\0%5\\0WMContentID\\0")
                .arg(setup_player_->value())
                .arg(enabled? 1 : 0)
                .arg(setup_artist_->value())
                .arg(setup_track_->value())
                .arg(setup_album_->value())
                ;
        msg.toWCharArray(wstr);

        COPYDATASTRUCT cd;
        cd.dwData = MSN_MAGIC_NUMBER;
        cd.lpData = wstr;
        cd.cbData = (lstrlenW(wstr) * 2) + 2;

        ::SendMessage(hwnd, WM_COPYDATA, 0, (LPARAM)&cd);
    }
}


