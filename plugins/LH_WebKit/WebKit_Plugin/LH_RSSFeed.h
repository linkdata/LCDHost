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

#ifndef LH_RSSFEED_H
#define LH_RSSFEED_H

#include <QNetworkAccessManager>
#include <QUrl>
#include <QWidget>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QList>
#include <QHash>

#include "LH_Qt_QString.h"
#include "LH_Qt_QSlider.h"
#include "LH_Qt_InputState.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_int.h"

#include <QObject>

#include "LH_QtObject.h"

struct RSSThumbnail
{
    QString url;
    int width;
    int height;
};

struct RSSItem
{
    QString title;
    QString link;
    QString description;
    QString author;
    QString pubDate;
    RSSThumbnail thumbnail;
};

class LH_RSSFeed: public QObject
{
    Q_OBJECT

    QTime notifyTimer_;

    QList<RSSItem> items_;
    int nowshowing_;
    int switchwait_;
    QXmlStreamReader xml_;
    QUrl url_;
    QDateTime lastrefresh_;
    QTime lastopen_;
    QTime lastmove_;

    QNetworkAccessManager *nam_;
    // int connectionId;

    void parseXml();

    int refresh_;
    int delay_;

    QList<QObject*> connections_;

public:
    int refresh();
    void setRefresh(int r);
    int delay();
    void setDelay(int d);
    QString url();
    void setUrl(QString u);

    void connect(QObject* intrfce);
    int connections();
    bool release(QObject* intrfce);

    LH_RSSFeed(QString feedUrl, int refreshTime = 5, int delayTime = 5);
    int notify(int code,void* param);

    RSSItem item(int mod = 0);

    void openItem(int mod = 0);
    void moveNext();
    void moveBack();

signals:
    void begin();
    void finished();
    void changed();
    void urlChanged();
    void refreshChanged();
    void delayChanged();

public slots:
    void fetch();
    void finished(QNetworkReply*);
};



extern QHash<QString, LH_RSSFeed* >* rssFeeds;

#endif // LH_RSSFEED_H
