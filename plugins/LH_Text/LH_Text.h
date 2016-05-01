/**
  \file     LH_Text.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

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

#ifndef LH_TEXT_H
#define LH_TEXT_H

#include <QtGlobal>
#include <QTextDocument>
#include <QString>
#include <QByteArray>
#include <QColor>
#include <QFont>
#include <QSize>
#include <QDateTime>

#include "LH_QtPlugin.h"
#include "LH_QtCFInstance.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_QString.h"
#include "LH_Qt_QFont.h"
#include "LH_Qt_QColor.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_QSlider.h"

#ifdef LH_TEXT_LIBRARY
# define LH_TEXT_EXPORT Q_DECL_EXPORT
#else
# define LH_TEXT_EXPORT Q_DECL_IMPORT
#endif

class LH_TEXT_EXPORT LH_Text : public LH_QtCFInstance
{
    Q_OBJECT
    QTextDocument doc_;
    QFont font_;
    QImage textimage_;
    QSizeF textsize_;
    int scrollposx_;
    int scrollposy_;
    bool richtext_;

protected:
    LH_Qt_QString *setup_text_;
    LH_Qt_QFont *setup_font_;
    LH_Qt_bool *setup_fontresize_;
    LH_Qt_QColor *setup_pencolor_;
    LH_Qt_QColor *setup_bgcolor_;
    LH_Qt_QStringList *setup_horizontal_;
    LH_Qt_QStringList *setup_vertical_;
    LH_Qt_QSlider *setup_scrollrate_;
    LH_Qt_QSlider *setup_scrollstep_;
    LH_Qt_QSlider *setup_scrollgap_;

    void setRenderHints( QPainter& p );

public:
    typedef enum
    {
        Automatic = 0,
        Percentage,
        Unscaled,
        Kilo,
        Mega,
        Giga,
        Tera
    } NumberMode;

    LH_Text(LH_QtObject* parent = 0);
    ~LH_Text();

    const char *userInit();

    int polling();
    int notify(int code,void* param);
    void prerender();
    int width( int forHeight = -1 );
    int height( int forWidth = -1 );
    QImage *render_qimage( int w, int h );
    QString text() const { return setup_text_->value(); }

    virtual QColor pencolor() { return setup_pencolor_->value(); }
    virtual QColor bgcolor() { return setup_bgcolor_->value(); }

    QFont font() const { return font_; }
    int horizontal() const { return setup_horizontal_->value(); }
    int vertical() const { return setup_vertical_->value(); }
    bool fontresize() const { return setup_fontresize_->value(); }
    int scrollrate() const { return setup_scrollrate_->value(); }
    int scrollstep() const { return setup_scrollstep_->value(); }
    int scrollgap() const { return setup_scrollgap_->value(); }

    void setPixelHeight( int height );
    bool setText( QString newText ); // return true if text changed
    bool setNum( qreal value, int mode = Automatic, bool suffix = true, qreal max = 0.0, bool bytes = false );
    QTextDocument& doc() { return doc_; }
    bool richtext() const { return richtext_; }
    QImage& textimage() { return textimage_; }
    QImage *prepareForRender(int w, int h);
    bool monochrome() const { return state() ? state()->dev_depth == 1 : false; }
    bool setStyleStrategy();

    static QImage makeImage(int w = 1, int h = 1)
    {
        QImage img(w,h,QImage::Format_ARGB32_Premultiplied);
        img.setDotsPerMeterX(3780);
        img.setDotsPerMeterY(3780);
        return img;
    }

    static QImage makeImage( QSize size )
    {
        return makeImage( size.width(), size.height() );
    }

public slots:
    void fontChanged();
    void textChanged();
    void makeTextImage( int forheight = 0 );
};

#endif // LH_TEXT_H
