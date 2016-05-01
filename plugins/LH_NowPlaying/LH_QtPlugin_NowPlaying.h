/**
  \file     LH_QtPlugin_NowPlaying.h
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


#ifndef LH_QTPLUGIN_NOWPLAYING_H
#define LH_QTPLUGIN_NOWPLAYING_H

#include <QDateTime>

#include "LH_QtPlugin.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_int.h"
#include "LH_Qt_InputState.h"
#include "LH_Qt_bool.h"
#include "Player.h"


#include "PlayerITunes.h"
#include "PlayerWinamp.h"
#include "PlayerFoobar.h"
#include "PlayerSpotify.h"
#include "PlayerVLC.h"
#include "PlayerWLM.h"

#include <QTime>
#include <QTimer>
#include <QDebug>
#include <QStringList>


extern CPlayer* player;
extern ArtworkCache* artworkCache;
extern bool isElevated;

class LH_QtPlugin_NowPlaying : public LH_QtPlugin
{
    enum elevationState
    {
        ELEVATION_UNKNOWN = -1,
        ELEVATION_NORMAL = 0,
        ELEVATION_ELEVATED = 1
    };

    typedef enum _TOKEN_ELEVATION_TYPE {
        TokenElevationTypeDefault = 1,
        TokenElevationTypeFull,
        TokenElevationTypeLimited
    } TOKEN_ELEVATION_TYPE, *PTOKEN_ELEVATION_TYPE;

    Q_OBJECT
    QTimer timer_;

#ifdef ITUNES_AUTO_CLOSING
    QTime elapsedTime_;
    bool forceClose_;
#endif
    HWND hWnd_iTunes_warn_cache_;
    HWND hWnd_Foobar_warn_cache_;

    QStringList priorities_;


    QString getWindowClass(LPCSTR windowCaption)
    {
        HWND hwnd = FindWindowA(NULL,windowCaption);
        if (hwnd != 0)
        {
            WCHAR winTitle[100];
            GetClassName(hwnd,winTitle,100);
            return QString::fromWCharArray(winTitle);
        } else
            return "Not found.";
    }

    bool playerControlCheck();

    elevationState GetElevationState(DWORD PID);

    QString getLastErrorMessage()
    {
        DWORD dwError;
        wchar_t errBuf[256];

        dwError = GetLastError();
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)errBuf, sizeof(errBuf),NULL);

        return QString("Err Code: %1 - %2").arg(dwError).arg(QString::fromWCharArray(errBuf));
    }

    void CloseWindow(HWND hWnd)
    {
        UINT uExitCode = 0;
        DWORD dwProcessId;
        GetWindowThreadProcessId( hWnd, &dwProcessId );
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, false, dwProcessId);
        if (hProcess != NULL)
            TerminateProcess(hProcess, uExitCode);
    }

public:
    QList<LH_Qt_int*> setup_priorities_;

    LH_Qt_QString *setup_page_playerConfig_;
    LH_Qt_QString *setup_page_keyBindings_;
    LH_Qt_QString *setup_page_remoteControl_;
    LH_Qt_QString *setup_page_advanced_;

    LH_Qt_QString *setup_heading_playerConfig_;
    LH_Qt_QString *setup_heading_playerConfig_break1_;
    QHash<QString, LH_Qt_bool*> setup_enable_players_;
    LH_Qt_QString *setup_vlc_port_;

    LH_Qt_QString *setup_heading_keyBindings_;
    LH_Qt_QString *setup_heading_keyBindings_break1_;
    LH_Qt_QString *setup_heading_keyBindings_break2_;
    LH_Qt_bool *setup_media_keys_VLC_;
    LH_Qt_InputState *setup_input_play_pause_;
    LH_Qt_InputState *setup_input_stop_;
    LH_Qt_InputState *setup_input_next_;
    LH_Qt_InputState *setup_input_prev_;
    LH_Qt_InputState *setup_input_shuffle_;
    LH_Qt_InputState *setup_input_repeat_;
    LH_Qt_InputState *setup_input_close_;

    LH_Qt_QString *setup_heading_remoteControl_;
    LH_Qt_QString *setup_control_play_pause_;
    LH_Qt_QString *setup_control_stop_;
    LH_Qt_QString *setup_control_next_;
    LH_Qt_QString *setup_control_prev_;
    LH_Qt_QString *setup_control_close_;
    LH_Qt_QString *setup_control_repeat_;
    LH_Qt_QString *setup_control_shuffle_;

    LH_Qt_QString *setup_heading_advanced_;
    LH_Qt_QString *setup_source_player_;
    LH_Qt_QString *setup_source_artist_;
    LH_Qt_QString *setup_source_album_;
    LH_Qt_QString *setup_source_title_;
    LH_Qt_QString *setup_source_status_;

    const char *userInit();
    void userTerm();
    void clearPlayer();

public slots:
    void refresh_data();
    void controlPlayPauseClick(QString key="",int flags=0,int value=0);
    void controlStopClick(QString key="",int flags=0,int value=0);
    void controlNextClick(QString key="",int flags=0,int value=0);
    void controlPrevClick(QString key="",int flags=0,int value=0);
    void controlCloseClick();
    void controlRepeatClick();
    void controlShuffleClick();
    void reorderPriorities();
    void updatePriorities();
    void selectPage(QObject *sender = NULL);
    void doneInitialize();

signals:
    void updated_data();
};

#endif // LH_QTPLUGIN_NOWPLAYING_H
