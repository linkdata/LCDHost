/**
  \file     LH_TextRSS.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009 Johan Lindh

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

#ifndef LH_TEXTRSS_H
#define LH_TEXTRSS_H

#include <QNetworkAccessManager>
#include <QUrl>
#include <QWidget>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QDateTime>

#include "LH_Text.h"
#include "../LH_Qt_QString.h"
#include "../LH_Qt_QSlider.h"
#include "../LH_Qt_InputState.h"
#include "../LH_Qt_QStringList.h"
#include "../LH_Qt_int.h"

class LH_TextRSS : public LH_Text
{
    Q_OBJECT
    QStringList titles_;
    QStringList links_;
    int nowshowing_;
    int switchwait_;
    QXmlStreamReader xml_;
    QUrl url_;
    QDateTime lastrefresh_;
    QTime lastopen_;

    QString currentTag;
    QString linkString;
    QString titleString;
    QNetworkAccessManager *nam_;
    int connectionId;

    void parseXml();

protected:
    LH_Qt_QString *setup_url_;
    LH_Qt_QSlider *setup_delay_;
    LH_Qt_QStringList *setup_method_;
    LH_Qt_InputState *setup_browser_;
    LH_Qt_InputState *setup_prev_;
    LH_Qt_InputState *setup_next_;
    LH_Qt_int *setup_refresh_;

public:
    LH_TextRSS(const char *name);

    int notify(int code,void* param);
    QImage *render_qimage( int w, int h );
    void setRssItem();

    static lh_class *classInfo();

public slots:
    void openBrowser(QString,int,int);
    void prevHeadline(QString,int,int);
    void nextHeadline(QString,int,int);
    void fetch();
    void finished(QNetworkReply*);
};

#endif // LH_TEXTRSS_H
