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

#include "LH_TS3CombinedStatusImage.h"

LH_PLUGIN_CLASS(LH_TS3CombinedStatusImage)

const char *LH_TS3CombinedStatusImage::userInit()
{
    if( const char *err = LH_QImage::userInit() ) return err;

    //setup_show_placeholder_->setTitle("Use default images");

    int LH_FLAG_SOURCEITEM =  LH_FLAG_HIDDEN | LH_FLAG_READONLY | LH_FLAG_NOSOURCE | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_AUTORENDER;

    setup_combined_status_ = new LH_Qt_QStringList(this, "Combined Status", QStringList() << "Not Running" << "Not Connected" << "No Speaker" << "Self Speaking" << "Other Speaking" << "Both Speaking", LH_FLAG_SOURCEITEM);

    setup_connection_status_ = new LH_Qt_QStringList(this, "Connection Status", QStringList() << "Not Running" << "Not Connected" << "Connected", LH_FLAG_SOURCEITEM);
    setup_connection_status_->setSubscribePath("/3rdParty/TeamSpeak 3/Connection Status");
    connect(setup_connection_status_, SIGNAL(changed()), this, SLOT(updateCombinedStatus()));

    setup_talking_ = new LH_Qt_QString(this, "Talking", "", LH_FLAG_SOURCEITEM);
    setup_talking_->setSubscribePath("/3rdParty/TeamSpeak 3/Speaking");
    connect(setup_talking_, SIGNAL(changed()), this, SLOT(updateCombinedStatus()));

    setup_talking_me_ = new LH_Qt_bool(this, "Me Talking", false, LH_FLAG_SOURCEITEM);
    setup_talking_me_->setSubscribePath("/3rdParty/TeamSpeak 3/Self Speaking");//, true);
    connect(setup_talking_me_, SIGNAL(changed()), this, SLOT(updateCombinedStatus()));

    setup_nickname_ = new LH_Qt_QString(this, "Nickname", "", LH_FLAG_SOURCEITEM);
    setup_nickname_->setSubscribePath("/3rdParty/TeamSpeak 3/Nickname");
    connect(setup_nickname_, SIGNAL(changed()), this, SLOT(updateCombinedStatus()));

    setup_mute_status_spk_ = new LH_Qt_QStringList(this, "Speaker Mute Status", QStringList() << "N/A" << "None" << "Muted" << "Active", LH_FLAG_SOURCEITEM);
    setup_mute_status_spk_->setSubscribePath("/3rdParty/TeamSpeak 3/Speaker Status");

    setup_mute_status_mic_ = new LH_Qt_QStringList(this, "Microphone Mute Status", QStringList() << "N/A" << "None" << "Muted" << "Active", LH_FLAG_SOURCEITEM);
    setup_mute_status_mic_->setSubscribePath("/3rdParty/TeamSpeak 3/Microphone Status");

    add_cf_target(setup_image_file_);
    add_cf_source(setup_combined_status_);
    add_cf_source(setup_mute_status_spk_);
    add_cf_source(setup_mute_status_mic_);

    return 0;
}

lh_class *LH_TS3CombinedStatusImage::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "3rdParty/TeamSpeak 3",
        "TS3CombinedStatusImage",
        "TS3 Combined Status (Image)",
        16,16,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };
    return &classInfo;
}

bool LH_TS3CombinedStatusImage::loadPlaceholderImage(QImage *img)
{
    QString status = setup_connection_status_->valueText();
    QString talking = setup_talking_->value();
    QString imageName = (
                    status=="Not Running"?"NotRunning":
                    status=="Not Connected"?"NotConnected":
                    talking==""?"NoSpeaker":
                    setup_talking_me_->value()==false?"OtherSpeaking":
                    talking==setup_nickname_->value()?"MeSpeaking":"BothSpeaking"
                    );
    return img->load(QString(":/images/LED_%1.png").arg(imageName));
}

void LH_TS3CombinedStatusImage::updateCombinedStatus()
{
    QString status = setup_connection_status_->valueText();
    QString talking = setup_talking_->value();
    QString combinedStatus = (
                status=="Not Running"?"Not Running":
                status=="Not Connected"?"Not Connected":
                talking==""?"No Speaker":
                setup_talking_me_->value()==false?"Other Speaking":
                talking==setup_nickname_->value()?"Self Speaking":"Both Speaking"
                );
    setup_combined_status_->setValue(combinedStatus);
}
