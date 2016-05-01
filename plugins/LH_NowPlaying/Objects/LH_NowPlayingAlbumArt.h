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

#ifndef LH_NOWPLAYINGALBUMART_H
#define LH_NOWPLAYINGALBUMART_H

#include "LH_QtInstance.h"
#include "LH_Qt_QFileInfo.h"
#include "../LH_QtPlugin_NowPlaying.h"

class LH_NowPlayingAlbumArt : public LH_QtInstance
{
    Q_OBJECT

protected:
    LH_Qt_QFileInfo *setup_file_;
    QString curAlbum_;
    QString curArtist_;
    QString curTitle_;

public:
    LH_NowPlayingAlbumArt();

    int polling() { return 0; }
    int notify(int n,void* p)  { Q_UNUSED(n); Q_UNUSED(p); return 0; }
    QImage *render_qimage( int w, int h );

    static lh_class *classInfo();

public slots:
    void refresh_image();
};

#endif // LH_NOWPLAYINGALBUMART_H
