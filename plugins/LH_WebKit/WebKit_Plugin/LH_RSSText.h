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

#ifndef LH_RSSTEXT_H
#define LH_RSSTEXT_H

#include "../LH_Text/LH_Text.h"
#include "LH_RSSInterface.h"

class LH_RSSText : public LH_Text
{
    Q_OBJECT

    LH_RSSInterface* rss_;

public:
    LH_RSSText();

    const char *userInit();
    int notify(int code,void* param);
    QImage *render_qimage( int w, int h );

    static lh_class *classInfo();

public slots:
    void setRssItem();
    void beginFetch();

};

#endif // LH_RSSTEXT_H
