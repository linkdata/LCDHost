/**
  \file     LH_NowPlayingText.cpp
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

#include "LH_NowPlayingText.h"

LH_PLUGIN_CLASS(LH_NowPlayingText)

LH_NowPlayingText::LH_NowPlayingText()
    : LH_Text()
    , setup_item_(0)
    , setup_custom_(0)
    , setup_hide_playing_state_(0)
{
    return;
}

const char *LH_NowPlayingText::userInit()
{
    if (const char* msg = LH_Text::userInit()) return msg;
    connect( lh_plugin(), SIGNAL(updated_data()), this, SLOT(refresh_text()) );

    setup_item_ = new LH_Qt_QStringList(this, "Item",
                                        QStringList()
                                        <<"Track Title"
                                        <<"Artist"
                                        <<"Album"
                                        <<"Duration"
                                        <<"Time Played"
                                        <<"Time Remaining"
                                        <<"Status"
                                        <<"Player"
                                        <<"Custom"
                                        <<"File Name"
                                        <<"Shuffle"
                                        <<"Repeat"
                                        <<"Rating"
                                        <<"Lyrics"
                                        ,
                                        LH_FLAG_FIRST
                                        );
    setup_item_->setValue(8);
    setup_item_->setHelp("The item to display<br/><br/>Note that not all players provide all data items. If you user such a player you may find a custom string a better choice.");
    connect(setup_item_, SIGNAL(changed()), this, SLOT(setup_item_changed()));

    setup_custom_ = new LH_Qt_QString(this, "^Custom", "{artist}{artist?: \"}{title}{artist?\"} {status?[}{status}{status?]}", LH_FLAG_FIRST);
    setup_custom_->setHelp("Enter a template string.<br/><br/>"
                           "Templates: <br/><br/>"
                           "<table>"
                           "<tr><td>Track Title:</td><td>{title}</td></tr>"
                           "<tr><td>Artist:</td><td>{artist}</td></tr>"
                           "<tr><td>Album:</td><td>{album}</td></tr>"
                           "<tr><td>Duration:</td><td>{duration}</td></tr>"
                           "<tr><td>Time Played</td><td>{played}</td></tr>"
                           "<tr><td>Time Remaining</td><td>{remaining}</td></tr>"
                           "<tr><td>Status</td><td>{status}</td></tr>"
                           "<tr><td>Player</td><td>{player}</td></tr>"
                           "</table><br/><br/>"
                           "Additionally you can include text conditionally using {<token>?<text>} , <br/>"
                           "e.g.: {album?Album: \"}{album}{album?\"} <br/>"
                           "This displays nothing when no album name is provided; when an album name is available however it will show: <br/>"
                           "Album: \"<album name>\"<br/>"
                           );
    connect(setup_custom_, SIGNAL(changed()), this, SLOT(refresh_text()));

    setup_hide_playing_state_ = new LH_Qt_bool(this, "Hide \"Playing\" State", false, LH_FLAG_FIRST);
    setup_hide_playing_state_->setHelp("This changes the behaviour of the \"State\" option, causing it not to display the \"Playing\" value.");
    connect(setup_hide_playing_state_, SIGNAL(changed()), this, SLOT(refresh_text()));

    setup_text_->setName( "Now Playing" );
    setup_text_->setFlag( LH_FLAG_READONLY, true );
    setup_text_->setFlag( LH_FLAG_HIDDEN, true );
    setup_text_->setFlag( LH_FLAG_NOSAVE_DATA, true );
    setup_text_->setFlag( LH_FLAG_NOSAVE_LINK, true );
    setText( "  " );
    return 0;
}

lh_class *LH_NowPlayingText::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "3rdParty/Music",
        "NowPlayingText",
        "Now Playing (Text)",
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
}

void LH_NowPlayingText::setup_item_changed()
{
    setup_custom_->setFlag(LH_FLAG_HIDDEN, setup_item_->value()==7);
    refresh_text();
}

void LH_NowPlayingText::refresh_text()
{
    //if (currentTrack->playerFound())
    if (player)
    {
        QString template_value = "";
        switch(setup_item_->value())
        {
        case 0:     //Track Title
            template_value = "{title}";
            break;
        case 1:     //Artist
            template_value = "{artist}";
            break;
        case 2:     //Album
            template_value = "{album}";
            break;
        case 3:     //Duration
            template_value = "{duration}";
            break;
        case 4:     //Time Played
            template_value = "{played}";
            break;
        case 5:     //Time Remaining
            template_value = "{remaining}";
            break;
        case 6:     //Status
            template_value = "{status}";
            break;
        case 7:     //Player
            template_value = "{player}";
            break;
        case 9:     //File
            template_value = "{file}";
            break;
        case 10:     //Shuffle
            template_value = "{shuffle}";
            break;
        case 11:     //Repeat
            template_value = "{repeat}";
            break;
        case 12:     //Rating
            template_value = "{rating}";
            break;
        case 13:     //Lyrics
            template_value = "{lyrics}";
            break;
        //case 8:     //Custom
        default:
            template_value = setup_custom_->value();
        }

        if( setText( player->replace_tokens(template_value, setup_hide_playing_state_->value()) ) )  callback(lh_cb_render,NULL);
    } else {
        // No player found
        if( setText( " " ) )  callback(lh_cb_render,NULL);
    }
}

