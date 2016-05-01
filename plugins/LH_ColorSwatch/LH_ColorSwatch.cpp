/**
  \file     LH_ColorSwatch.cpp
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

#include "LH_ColorSwatch.h"

#include <climits>
#include <cfloat>
#include <QDebug>

LH_PLUGIN_CLASS(LH_ColorSwatch)

lh_class *LH_ColorSwatch::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Static",
        "ColorSwatch",
        "Color Swatch",
        24,24,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

LH_ColorSwatch::LH_ColorSwatch(qreal lowVal, QColor lowCol,
               qreal highVal, QColor highCol,
               bool lockDown) :
    LH_Rectangle(),
    lowVal_(lowVal),
    lowCol_(lowCol),
    highVal_(highVal),
    highCol_(highCol),
    lockDown_(lockDown),
    setup_lowerbound_color_(0),
    setup_lowerbound_value_(0),
    setup_upperbound_color_(0),
    setup_upperbound_value_(0),
    setup_current_color_(0),
    setup_current_value_(0)
{
}

const char *LH_ColorSwatch::userInit()
{
    if( const char *err = LH_Rectangle::userInit() ) return err;

    setup_penwidth_->setFlags(LH_FLAG_READONLY | LH_FLAG_HIDDEN | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    setup_penwidth_->setValue(1);
    setup_rounding_->setFlags(LH_FLAG_READONLY | LH_FLAG_HIDDEN | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    setup_rounding_->setValue(1);
    setup_pencolor_->setFlags(LH_FLAG_READONLY | LH_FLAG_HIDDEN | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    setup_pencolor_->setValue(Qt::black);
    setup_bgcolor1_->setFlags(LH_FLAG_READONLY | LH_FLAG_HIDDEN | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    setup_bgcolor1_->setValue(Qt::black);
    setup_bgcolor2_->setFlags(LH_FLAG_READONLY | LH_FLAG_HIDDEN | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    setup_bgcolor2_->setValue(Qt::black);
    setup_gradient_->setFlags(LH_FLAG_READONLY | LH_FLAG_HIDDEN | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    setup_gradient_->setValue(false);
    setup_horizontal_->setFlags(LH_FLAG_READONLY | LH_FLAG_HIDDEN | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK);
    setup_horizontal_->setValue(false);

    setup_lowerbound_value_ = new LH_Qt_float(
                this,"Lower Bound Value", lowVal_,
                FLT_MIN, FLT_MAX, LH_FLAG_AUTORENDER);
    setup_lowerbound_color_ = new LH_Qt_QColor(this, "Lower Bound Color", lowCol_, LH_FLAG_AUTORENDER);

    setup_upperbound_value_ = new LH_Qt_float(
                this,"Upper Bound Value", highVal_,
                FLT_MIN, FLT_MAX, LH_FLAG_AUTORENDER);
    setup_upperbound_color_ = new LH_Qt_QColor(this, "Upper Bound Color", highCol_, LH_FLAG_AUTORENDER);

    setup_current_value_ = new LH_Qt_float(this,"Current Value", lowVal_);//, std::numeric_limits<qreal>::min(), std::numeric_limits<qreal>::max(), LH_FLAG_AUTORENDER);
    setup_current_color_ = new LH_Qt_QColor(this, "Current Color", lowCol_, LH_FLAG_AUTORENDER | LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSINK );

    if(lockDown_)
        setup_current_value_->setFlag(LH_FLAG_READONLY, true);

    connect( setup_lowerbound_color_, SIGNAL(changed()), this, SLOT(updateColor()) );
    connect( setup_lowerbound_value_, SIGNAL(changed()), this, SLOT(updateColor()) );
    connect( setup_upperbound_color_, SIGNAL(changed()), this, SLOT(updateColor()) );
    connect( setup_upperbound_value_, SIGNAL(changed()), this, SLOT(updateColor()) );
    connect( setup_current_color_, SIGNAL(changed()), this, SLOT(updateColor()) );
    connect( setup_current_value_, SIGNAL(changed()), this, SLOT(updateColor()) );

    connect( setup_lowerbound_color_, SIGNAL(set()), this, SLOT(updateColor()) );
    connect( setup_lowerbound_value_, SIGNAL(set()), this, SLOT(updateColor()) );
    connect( setup_upperbound_color_, SIGNAL(set()), this, SLOT(updateColor()) );
    connect( setup_upperbound_value_, SIGNAL(set()), this, SLOT(updateColor()) );
    connect( setup_current_color_, SIGNAL(set()), this, SLOT(updateColor()) );
    connect( setup_current_value_, SIGNAL(set()), this, SLOT(updateColor()) );

    add_cf_target(setup_lowerbound_value_);
    add_cf_target(setup_lowerbound_color_);
    add_cf_target(setup_upperbound_value_);
    add_cf_target(setup_upperbound_color_);
    add_cf_source(setup_current_value_);

    return 0;
}

void LH_ColorSwatch::updateColor()
{
    int r_lower = setup_lowerbound_color_->value().red();
    int r_delta = setup_upperbound_color_->value().red() - setup_lowerbound_color_->value().red();

    int g_lower = setup_lowerbound_color_->value().green();
    int g_delta = setup_upperbound_color_->value().green() - setup_lowerbound_color_->value().green();

    int b_lower = setup_lowerbound_color_->value().blue();
    int b_delta = setup_upperbound_color_->value().blue() - setup_lowerbound_color_->value().blue();

    int a_lower = setup_lowerbound_color_->value().alpha();
    int a_delta = setup_upperbound_color_->value().alpha() - setup_lowerbound_color_->value().alpha();

    qreal low = setup_lowerbound_value_->value();
    qreal high = setup_upperbound_value_->value();
    qreal value = setup_current_value_->value();

    if(high < low)
    {
        qreal temp = low;
        low = high;
        high = temp;
    }
    if(value < low) value = low;
    if(value > high) value = high;

    qreal position = (high-low==0? 0.5 : (value-low) / (high-low));
    int r_new = qRound(r_lower + r_delta * position);
    int g_new = qRound(g_lower + g_delta * position);
    int b_new = qRound(b_lower + b_delta * position);
    int a_new = qRound(a_lower + a_delta * position);

    QColor new_col = QColor::fromRgb(r_new, g_new, b_new, a_new);

    setup_current_color_->setValue(new_col);
    setup_bgcolor1_->setValue(new_col);
    requestRender();
}
