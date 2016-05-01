/**
  \file     LH_ColorSwatch.h
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2010,2011 Andy Bridges & Johan Lindh
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

 **/

#ifndef LH_COLORSWATCH_H
#define LH_COLORSWATCH_H

#include "../LH_Decor/LH_Rectangle.h"

#include "LH_Qt_QColor.h"
#include "LH_Qt_float.h"

#ifdef LH_COLORSWATCH_LIBRARY
# define LH_COLORSWATCH_EXPORT Q_DECL_EXPORT
#else
# define LH_COLORSWATCH_EXPORT Q_DECL_IMPORT
#endif

class LH_COLORSWATCH_EXPORT LH_ColorSwatch : public LH_Rectangle
{
    Q_OBJECT

    qreal lowVal_;
    QColor lowCol_;
    qreal highVal_;
    QColor highCol_;
    bool lockDown_;

protected:
    LH_Qt_QColor *setup_lowerbound_color_;
    LH_Qt_float *setup_lowerbound_value_;
    LH_Qt_QColor *setup_upperbound_color_;
    LH_Qt_float *setup_upperbound_value_;
    LH_Qt_QColor *setup_current_color_;
    LH_Qt_float *setup_current_value_;

public:
    LH_ColorSwatch(qreal lowVal = 0, QColor lowCol = Qt::white,
                   qreal highVal = 100, QColor highCol = Qt::black,
                   bool lockDown = false);
    const char *userInit();
    static lh_class *classInfo();
    void setValue(qreal val) { setup_current_value_->setValue(val); }

public slots:
    void updateColor();
};

#endif // LH_COLORSWATCH_H
