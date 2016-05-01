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

#ifndef LH_NOWPLAYINGREMOTECONTROL_H
#define LH_NOWPLAYINGREMOTECONTROL_H

#include <QDateTime>
#include <stdio.h>
#include <windows.h>
#include <shellapi.h>

#include <LH_QtInstance.h>
#include <LH_Qt_InputState.h>
#include <LH_Qt_QFileInfo.h>
#include "../LH_QtPlugin_NowPlaying.h"

class LH_NowPlayingRemoteControl : public LH_QtInstance
{
    Q_OBJECT
    QString getLastErrorMessage()
    {
        DWORD dwError;
        wchar_t errBuf[256];

        dwError = GetLastError();
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)errBuf, sizeof(errBuf),NULL);

        return QString("Err Code: %1 - %2").arg(dwError).arg(QString::fromWCharArray(errBuf));
    }

protected:
    LH_Qt_InputState *setup_input_play_pause_;
    LH_Qt_InputState *setup_input_stop_;
    LH_Qt_InputState *setup_input_next_;
    LH_Qt_InputState *setup_input_prev_;
    LH_Qt_InputState *setup_input_shuffle_;
    LH_Qt_InputState *setup_input_repeat_;
    LH_Qt_InputState *setup_input_close_;
    LH_Qt_InputState *setup_input_open_;
    LH_Qt_QFileInfo *setup_player_file_;

public:
    LH_NowPlayingRemoteControl();
    const char *userInit(){ hide(); return NULL; }
    static lh_class *classInfo();

public slots:
    void controlPlayPauseClick();
    void controlStopClick();
    void controlNextClick();
    void controlPrevClick();
    void controlCloseClick();
    void controlOpenClick();
    void controlRepeatClick();
    void controlShuffleClick();

};

#endif // LH_NOWPLAYINGREMOTECONTROL_H
