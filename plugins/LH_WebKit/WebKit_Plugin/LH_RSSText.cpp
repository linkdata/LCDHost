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

#include "LH_RSSText.h"
#include <QPainter>
#include <QDebug>

LH_PLUGIN_CLASS(LH_RSSText)

lh_class *LH_RSSText::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Dynamic",
        "DynamicRSSFeedText",
        "RSS feed (Text)",
        -1, -1,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };
#if 0
    if( classInfo.width == -1 )
    {
        QFont arial10("Arial",10);
        QFontMetrics fm( arial10 );
        classInfo.height = fm.height();
        classInfo.width = classInfo.height * 4;
    }
#endif
    return &classInfo;
}

LH_RSSText::LH_RSSText()
    : LH_Text()
    , rss_(0)
{
}

const char *LH_RSSText::userInit()
{
    if(const char *err = LH_Text::userInit()) return err;
    rss_ = new LH_RSSInterface(this);
    connect( rss_, SIGNAL(changed()), this, SLOT(setRssItem()) );
    connect( rss_, SIGNAL(begin()), this, SLOT(beginFetch()) );
    setup_horizontal_->setFlag( LH_FLAG_HIDDEN, true );
    setup_vertical_->setFlag( LH_FLAG_HIDDEN, true );
    setup_scrollrate_->setFlag( LH_FLAG_HIDDEN, true );
    setup_scrollstep_->setFlag( LH_FLAG_HIDDEN, true );
    setup_text_->setFlag( LH_FLAG_HIDDEN, true );
    setup_text_->setFlag( LH_FLAG_NOSAVE_DATA, true );
    setup_text_->setFlag( LH_FLAG_NOSAVE_LINK, true );
    setup_text_->setFlag( LH_FLAG_READONLY, true );
    setRssItem();
    return 0;
}

int LH_RSSText::notify(int code,void* param)
{
    return rss_->notify(code,param) | LH_Text::notify(code,param);
}

void LH_RSSText::setRssItem()
{
    if( setText(rss_->item().title) ) requestRender();
}


QImage *LH_RSSText::render_qimage( int w, int h )
{
    if(QImage *img = prepareForRender(w, h))
    {
        QPainter painter;
        if( painter.begin(img) )
        {
            QRectF target;

            target.setSize( textimage().size() );
            target.moveLeft( img->width()/2 - textimage().width()/2 );
            target.moveTop( img->height()/2 - textimage().height()/2 );

            if( textimage().width() > img->width() )
                target.moveLeft( 0 );

            painter.drawImage( target, textimage(), textimage().rect() );
            painter.end();
        }

        return img;
    }

    return 0;
}


void LH_RSSText::beginFetch()
{
    setText("Loading RSS feed");
}
