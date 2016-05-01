/**
  \file     LH_Background.cpp
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

#include <QDebug>
#include <QPainter>

#include "LH_Background.h"

LH_PLUGIN_CLASS(LH_Background)

lh_class *LH_Background::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Static",
        "StaticBackground",
        "Background",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

LH_Background::LH_Background()
    : LH_QtInstance()
    , setup_startcolor_(0)
    , setup_stopcolor_(0)
    , setup_gradient_(0)
    , setup_horizontal_(0)
{
}

const char *LH_Background::userInit() {
    if( const char *err = LH_QtInstance::userInit() ) return err;
    setup_startcolor_ = new LH_Qt_QColor(this,tr("First color"),Qt::white,LH_FLAG_AUTORENDER);
    setup_gradient_ = new LH_Qt_bool(this,tr("^Background is a gradient"),false);
    connect( setup_gradient_, SIGNAL(change(bool)), this, SLOT(enableGradient(bool)) );
    setup_stopcolor_ = new LH_Qt_QColor(this,tr("Second color"),Qt::lightGray,LH_FLAG_HIDDEN|LH_FLAG_AUTORENDER);
    setup_horizontal_ = new LH_Qt_bool(this,tr("^Gradient is horizontal"),false,LH_FLAG_HIDDEN|LH_FLAG_AUTORENDER);
    return 0;
}

void LH_Background::enableGradient(bool b)
{
    setup_stopcolor_->setFlag( LH_FLAG_HIDDEN, !b );
    setup_horizontal_->setFlag( LH_FLAG_HIDDEN, !b );
    requestRender();
}

QImage *LH_Background::render_qimage( int w, int h )
{
    if(QImage *img = initImage(w,h))
    {
        img->fill( qRgba(0,0,0,0) );

        QPainter painter;
        if( painter.begin(img) )
        {
            painter.setPen(Qt::NoPen);
            if( setup_gradient_->value() )
            {
                QLinearGradient gradient;
                gradient.setStart(0,0);
                if( setup_horizontal_->value() ) gradient.setFinalStop(img->width(),0);
                else gradient.setFinalStop(0,img->height());
                gradient.setColorAt(0, setup_startcolor_->value() );
                gradient.setColorAt(1, setup_stopcolor_->value() );
                QBrush brush(gradient);
                painter.setBrush( brush );
            }
            else
                painter.setBrush( setup_startcolor_->value() );
            painter.drawRect( img->rect() );
            painter.end();
        }
        return img;
    }
    return 0;
}
