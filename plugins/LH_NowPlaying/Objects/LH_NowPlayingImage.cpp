/**
  \file     LH_NowPlayingImage.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010 Andy Bridges
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#include "LH_NowPlayingImage.h"

#include <QtGlobal>
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QString>
#include <QRegExp>
#include <QHash>

#include "LH_Qt_QStringList.h"

LH_PLUGIN_CLASS(LH_NowPlayingImage)

lh_class *LH_NowPlayingImage::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "3rdParty/Music",
        "NowPlayingStatusImage",
        "Now Playing (Status Image)",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

LH_NowPlayingImage::LH_NowPlayingImage() : LH_QImage()
{}

const char *LH_NowPlayingImage::userInit()
{
    if( const char *err = LH_QImage::userInit() ) return err;


    setup_image_file_->setFlags( LH_FLAG_AUTORENDER | LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA );

    setup_item_ = new LH_Qt_QStringList(this, "Item",
                                        QStringList()
                                        <<"Status"
                                        <<"Player"
                                        <<"Shuffle"
                                        <<"Repeat"
                                        <<"Rating"
                                        ,
                                        0
                                        );
    setup_item_->setHelp("The item to display<br/><br/>Note that not all players provide all data items. If you user such a player you may find a custom string a better choice.");

    setup_value_ = new LH_Qt_QString(this, "Current Value","", LH_FLAG_READONLY);

    connect( lh_plugin(), SIGNAL(updated_data()), this, SLOT(updateValue()) );
    connect(setup_item_, SIGNAL(changed()), this, SLOT(updateValue()));

    add_cf_target(setup_image_file_);
    add_cf_source(setup_value_);

    return 0;
}

bool LH_NowPlayingImage::loadPlaceholderImage(QImage *img)
{
    QString placeholderName = "";
    QString playerName = player->GetPlayer();
    qreal ratingValue = 0;
    switch(setup_item_->value())
    {
    case 0:     //Status
        if (playerName=="")
            placeholderName="power_off";
        else
            switch(player->GetState())
            {
            case PLAYER_PLAYING:
                placeholderName = "play";
                break;
            case PLAYER_PAUSED:
                placeholderName = "pause";
                break;
            default:
                placeholderName = "stop";
                break;
            }
        placeholderName = QString("control_%1_blue").arg(placeholderName);
        break;
    case 1:     //Player
        placeholderName="none";
        if(playerName.startsWith("iTunes"))
            placeholderName="iTunes";
        else
        if(playerName.startsWith("Winamp"))
            placeholderName="winamp";
        else
        if(playerName.startsWith("Foobar"))
            placeholderName="foobar2000";
        else
        if(playerName.startsWith("Spotify"))
            placeholderName="spotify";
        else
        if(playerName.startsWith("VLC"))
            placeholderName="vlc";
        else
        if(playerName.startsWith(CPlayerWLM::WLMPlayerName()))
            placeholderName="wmp"; // No image available for WLM-based stuff obviously, so use the WMP icon

        placeholderName = QString("player_%1").arg(placeholderName);
        break;
    case 2:     //Shuffle
        if(player->GetShuffle()) placeholderName = "shuffle"; else placeholderName = "shuffle_off";
        placeholderName = QString("control_%1_blue").arg(placeholderName);
        break;
    case 3:     //Repeat
        if(player->GetRepeat()) placeholderName = "repeat"; else placeholderName = "repeat_off";
        placeholderName = QString("control_%1_blue").arg(placeholderName);
        break;
    case 4:     //Rating
        ratingValue = (qreal)player->GetRating();
        if(ratingValue<0.25) placeholderName = "0.0"; else
        if(ratingValue<0.75) placeholderName = "0.5"; else
        if(ratingValue<1.25) placeholderName = "1.0"; else
        if(ratingValue<1.75) placeholderName = "1.5"; else
        if(ratingValue<2.25) placeholderName = "2.0"; else
        if(ratingValue<2.75) placeholderName = "2.5"; else
        if(ratingValue<3.25) placeholderName = "3.0"; else
        if(ratingValue<3.75) placeholderName = "3.5"; else
        if(ratingValue<4.25) placeholderName = "4.0"; else
        if(ratingValue<4.75) placeholderName = "4.5"; else
                             placeholderName = "5.0";
        break;
    default:
        placeholderName = "";
        break;
    }

    if(placeholderName=="")
        return LH_QImage::loadPlaceholderImage(img);
    return img->load(QString(":/images/%1.png").arg(placeholderName));
}

void LH_NowPlayingImage::updateValue()
{
    if (player)
    {
        QString template_value = "";
        switch(setup_item_->value())
        {
        case 0:     //Status
            template_value = "{status}";
            break;
        case 1:     //Player
            template_value = "{player}";
            break;
        case 2:     //Shuffle
            template_value = "{shuffle}";
            break;
        case 3:     //Repeat
            template_value = "{repeat}";
            break;
        case 4:     //Rating
            template_value = "{rating}";
            break;
        }
        setup_value_->setValue( player->replace_tokens(template_value) ) ;
        callback(lh_cb_render, NULL);
    } else {
        // No player found
        setup_value_->setValue( "" ) ;
    }

}
