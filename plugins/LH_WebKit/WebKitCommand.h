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

#ifndef WEBKITCOMMAND_H
#define WEBKITCOMMAND_H

#include <QString>
#include <QUrl>
#include <QSize>
#include <QByteArray>
#include <QImage>
#include <QIODevice>

class WebKitCommand
{
public:
    WebKitCommand( int c, QSize s = QSize(), QUrl u = QUrl(), QString h = QString() );
    WebKitCommand( QByteArray a ) : command_(0), size_(QSize()), url_(QUrl()), html_(QString()) {decode(a);}

    int command() const { return command_; }
    QSize size() const { return size_; }
    QUrl url() const { return url_; }
    QString html() const { return html_; }

    QByteArray encode();
    bool decode( const QByteArray& );
    bool write( QIODevice * );

private:
    int command_; // 'HURQ'
    QSize size_; // size to render into
    QUrl url_; // if html isEmpty(), this is URL to load, otherwise it's baseURL
    QString html_; // leave empty if URL request
};

typedef struct WebKitData_
{
    qint32 bytecount; // number of bytes of image data that follows
    int progress;
    int w, h;
} WebKitData;

class WebKitResult
{
public:
    WebKitResult( int p = 0, QImage i = QImage() ) : progress_(p), image_(i) {}

    int progress() const { return progress_; }
    QImage image() const { return image_; }

    QByteArray encode();
    bool decode( const QByteArray& );

private:
    int progress_;
    QImage image_;
};

#endif // WEBKITCOMMAND_H
