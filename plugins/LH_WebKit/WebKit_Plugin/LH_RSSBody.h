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

#ifndef LH_RSSBODY_H
#define LH_RSSBODY_H

#include "LH_WebKit.h"
#include "WebKitCommand.h"
#include "LH_RSSInterface.h"

class LH_RSSBody : public LH_WebKit
{
    Q_OBJECT

    LH_RSSInterface rss_;

protected:
    QHash<QString, QString> getTokens();

public:
    LH_RSSBody();

    const char *userInit() { setRssItem(); return 0; }

    int notify(int code,void* param);

    static lh_class *classInfo();

public slots:
    void setRssItem();
    void beginFetch();

};

#endif // LH_RSSBODY_H
