/**
  \file     LH_Rectangle.h
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

#ifndef LH_RECTANGLE_H
#define LH_RECTANGLE_H

#include "LH_QtPlugin.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_QSlider.h"
#include "LH_Qt_QColor.h"

#include "LH_Decor.h"
#include "LH_QtCFInstance.h"

class LH_DECOR_EXPORT LH_Rectangle : public LH_QtCFInstance
{
    Q_OBJECT

protected:
    LH_Qt_QSlider *setup_penwidth_;
    LH_Qt_QSlider *setup_rounding_;
    LH_Qt_QColor *setup_pencolor_;
    LH_Qt_QColor *setup_bgcolor1_;
    LH_Qt_QColor *setup_bgcolor2_;
    LH_Qt_bool *setup_gradient_;
    LH_Qt_bool *setup_horizontal_;

public:
    LH_Rectangle(LH_QtObject* parent = 0);

    const char *userInit();
    virtual int notify( int code,void* param );
    QImage *render_qimage( int w, int h );

    int penwidth() const { return setup_penwidth_->value(); }
    int rounding() const { return setup_rounding_->value(); }
    bool gradient() const { return setup_gradient_->value(); }
    bool horizontal() const { return setup_horizontal_->value(); }
    QColor pencolor() const { return setup_pencolor_->value(); }
    QColor bgcolor1() const { return setup_bgcolor1_->value(); }
    QColor bgcolor2() const { return setup_bgcolor2_->value(); }

public slots:
    void enableGradient(bool);
};

#endif // LH_RECTANGLE_H
