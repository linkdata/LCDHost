/**
  \file     LH_DataViewerText.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010 Andy Bridges
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

  */

#include <QFont>
#include <QFontMetrics>
#include <QTime>
#include <QRegExp>
#include <QDebug>

#include "LH_DataViewerText.h"

LH_PLUGIN_CLASS(LH_DataViewerText)

lh_class *LH_DataViewerText::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Dynamic/DataViewer",
        "DataViewerText",
        "Data Text",
        -1, -1,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };
#if 0
    if( classInfo.width == -1 )
    {
        QFont font;
        QFontMetrics fm( font );
        classInfo.height = fm.height();
        classInfo.width = fm.width("100%");
    }
#endif
    return &classInfo;
}

const char *LH_DataViewerText::userInit()
{
    if( const char *err = LH_Text::userInit() ) return err;
    setup_text_->setValue(" ");
    setup_text_->setFlags(LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA);

    setup_lookup_code_ = new LH_Qt_QString( this, "Data Template", "", LH_FLAG_AUTORENDER);
    setup_lookup_code_->setOrder(-3);

    setup_item_name_ = new LH_Qt_QString( this, "Data Names" , "", LH_FLAG_READONLY | LH_FLAG_NOSAVE_DATA);
    setup_item_name_->setOrder(-3);

    connect( setup_lookup_code_, SIGNAL(changed()), this, SLOT(updateText()) );

    updateTimer_.start();
    scrollTimer_.start();
    setText("");
    setText(" ");
    callback(lh_cb_render,NULL);
    return 0;
}

int LH_DataViewerText::polling()
{
    if(updateTimer_.elapsed()>=polling_rate)
    {
        updateText();
        updateTimer_.restart();
    }
    if(scrollTimer_.elapsed()>=scroll_poll_)
    {
        scroll_poll_ = LH_Text::polling();
        scrollTimer_.restart();
    }
    return (scroll_poll_<polling_rate && scroll_poll_!=0? scroll_poll_ : polling_rate);
}

void LH_DataViewerText::updateText()
{
    if( data_.open() && data_.valid(setup_lookup_code_->value()))
    {
        setup_item_name_->setValue( data_.populateLookupCode(setup_lookup_code_->value(), true) );
        QString txtVal = data_.populateLookupCode(setup_lookup_code_->value(), false, true);
        if( setText( txtVal ) ) callback(lh_cb_render,NULL);
    }
}
