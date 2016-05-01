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

#ifndef LH_RSSINTERFACE_H
#define LH_RSSINTERFACE_H

#include <QNetworkAccessManager>
#include <QUrl>
#include <QWidget>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QList>

#include "LH_Qt_QString.h"
#include "LH_Qt_QSlider.h"
#include "LH_Qt_InputState.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_int.h"

#include <QObject>

#include "LH_QtObject.h"
#include "LH_RSSFeed.h"


class LH_RSSInterface: public QObject
{
    Q_OBJECT

    QString url_;

    void connectRSS();
    void releaseRSS();
    QString getDefaultURL();

protected:
    LH_Qt_QString *setup_url_;
    LH_Qt_QSlider *setup_delay_;
    LH_Qt_QStringList *setup_method_;
    LH_Qt_InputState *setup_browser_;
    LH_Qt_InputState *setup_prev_;
    LH_Qt_InputState *setup_next_;
    LH_Qt_int *setup_refresh_;
    LH_Qt_int *setup_modify_;

    LH_RSSFeed* getFeed();
public:
    LH_RSSInterface(LH_QtObject* parent);
    ~LH_RSSInterface();

    int notify(int code,void* param);

    RSSItem item();

signals:
    void begin();
    void finished();
    void changed();

public slots:
    void reemitBegin(){ emit begin(); }
    void reemitFinished(){ emit finished(); }
    void reemitChanged(){ emit changed(); }

    void changeURL();
    void changeDelay();
    void updateDelay();
    void changeRefresh();
    void updateRefresh();

    void openBrowser(QString,int,int);
    void prevHeadline(QString,int,int);
    void nextHeadline(QString,int,int);
};

#endif // LH_RSSINTERFACE_H
