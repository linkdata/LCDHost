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

#include "LH_NowPlayingRemoteControl.h"
#include <QRegExp>
#include <QProcess>

LH_PLUGIN_CLASS(LH_NowPlayingRemoteControl)

lh_class *LH_NowPlayingRemoteControl::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "3rdParty/Music",
        "NowPlayingRemoteControl",
        "Remote Control",
        -1, -1,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

LH_NowPlayingRemoteControl::LH_NowPlayingRemoteControl()
{

    setup_input_play_pause_ = new LH_Qt_InputState(this, "Play & Pause", "", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    connect( setup_input_play_pause_, SIGNAL(input(QString,int,int)), this, SLOT(controlPlayPauseClick()) );

    setup_input_stop_ = new LH_Qt_InputState(this, "Stop", "", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    connect( setup_input_stop_, SIGNAL(input(QString,int,int)), this, SLOT(controlStopClick()) );

    setup_input_next_ = new LH_Qt_InputState(this, "Next Track", "", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    connect( setup_input_next_, SIGNAL(input(QString,int,int)), this, SLOT(controlNextClick()) );

    setup_input_prev_ = new LH_Qt_InputState(this, "Previous Track", "", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    connect( setup_input_prev_, SIGNAL(input(QString,int,int)), this, SLOT(controlPrevClick()) );

    setup_input_shuffle_ = new LH_Qt_InputState(this, "Shuffle", "", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    connect( setup_input_shuffle_, SIGNAL(input(QString,int,int)), this, SLOT(controlShuffleClick()) );

    setup_input_repeat_ = new LH_Qt_InputState(this, "Repeat", "", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    connect( setup_input_repeat_, SIGNAL(input(QString,int,int)), this, SLOT(controlRepeatClick()) );

    setup_input_close_ = new LH_Qt_InputState(this, "Close Player", "", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    connect( setup_input_close_, SIGNAL(input(QString,int,int)), this, SLOT(controlCloseClick()) );

    setup_input_open_ = new LH_Qt_InputState(this, "Open Player", "", LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    connect( setup_input_open_, SIGNAL(input(QString,int,int)), this, SLOT(controlOpenClick()) );

    setup_player_file_ = new LH_Qt_QFileInfo(this, "Player Application", QFileInfo(), LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
}

void LH_NowPlayingRemoteControl::controlPlayPauseClick()
{
    if(!player)
        return;
    if(player->GetState()==PLAYER_PLAYING)
        player->Pause();
    else
        player->Play();
}

void LH_NowPlayingRemoteControl::controlStopClick()
{
    if(player)
        player->Stop();
}

void LH_NowPlayingRemoteControl::controlNextClick()
{
    if(player)
        player->Next();
}

void LH_NowPlayingRemoteControl::controlPrevClick()
{
    if(player)
        player->Previous();
}

void LH_NowPlayingRemoteControl::controlCloseClick()
{
    if(player)
        player->ClosePlayer();
}

void LH_NowPlayingRemoteControl::controlRepeatClick()
{
    if(player)
        player->SetRepeat(!player->GetRepeat());
}

void LH_NowPlayingRemoteControl::controlShuffleClick()
{
    if(player)
       player->SetShuffle(!player->GetShuffle());
}

void LH_NowPlayingRemoteControl::controlOpenClick()
{
    QFileInfo exe = setup_player_file_->value();
    //qDebug() << "Elevation? " << isElevated;
    if(!exe.isFile())
        exe = QFileInfo(QString("%1%2").arg(state()->dir_layout).arg(exe.fileName()));

    if(!exe.isFile())
    {
        qWarning() << "LH_NowPlaying: Unable to find player application. Launch aborted.";
    } else {
        if(isElevated)
            ShellExecuteA(0, "runas", exe.absoluteFilePath().toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
        else
            ShellExecuteA(0, "open", exe.absoluteFilePath().toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
    }
}

