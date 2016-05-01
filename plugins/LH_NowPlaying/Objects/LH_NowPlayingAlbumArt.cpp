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

#include "LH_NowPlayingAlbumArt.h"
#include <QDebug>

LH_PLUGIN_CLASS(LH_NowPlayingAlbumArt)

LH_NowPlayingAlbumArt::LH_NowPlayingAlbumArt()
{
    connect( lh_plugin(), SIGNAL(updated_data()), this, SLOT(refresh_image()) );

    setup_file_ = new LH_Qt_QFileInfo( this, tr("Image"), QFileInfo(), LH_FLAG_AUTORENDER | LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA);
    refresh_image();
    //setup_file_->setFlag( LH_FLAG_HIDDEN, true );
    return;
}

lh_class *LH_NowPlayingAlbumArt::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "3rdParty/Music",
        "NowPlayingAlbumArt",
        "Now Playing (Album Art)",
        -1, -1,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

QImage *LH_NowPlayingAlbumArt::render_qimage(int w, int h)
{
    if(QImage *img = initImage(w, h))
    {
        if(setup_file_->value().isFile())
        {
            if(!img->load(setup_file_->value().absoluteFilePath()))
                qDebug() << "Cannot load image: " << setup_file_->value().absoluteFilePath();
        }
        return img;
    }
    return 0;
}

void LH_NowPlayingAlbumArt::refresh_image()
{
    if(player!=NULL)
    {
        QFileInfo f(player->GetCoverPath());
        if(setup_file_->value().absoluteFilePath() != f.absoluteFilePath() ||
                player->GetAlbum() != curAlbum_ ||
                player->GetArtist() != curArtist_ ||
                player->GetTitle() != curTitle_
                )
        {
            curAlbum_ = player->GetAlbum();
            curArtist_ = player->GetArtist();
            curTitle_ = player->GetTitle();
            setup_file_->setValue(f);
            this->requestRender();
        }
    }
}
