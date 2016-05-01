/**
  \file     LH_MonitoringText.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  \legalese
    This module is based on original work by Johan Lindh and uses code freely
    available from http://allthingsgeek.wordpress.com/ by "Noccy"

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

#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QTime>
#include <QRegExp>

#include "LH_MonitoringText.h"

LH_PLUGIN_CLASS(LH_MonitoringText)

lh_class *LH_MonitoringText::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        STRINGIZE(MONITORING_FOLDER),
        STRINGIZE(COMMON_OBJECT_NAME)"Text",
        STRINGIZE(COMMON_OBJECT_NAME)" (Text)",
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

LH_MonitoringText::LH_MonitoringText()
    : LH_Text()
    , LH_MonitoringObject(this, mdmAll, false, true)
    , setup_value_round_(0)
    , setup_append_units_(0)
    , setup_pre_text_(0)
    , setup_post_text_(0)
{
}

const char *LH_MonitoringText::userInit()
{
    if( const char *err = LH_Text::userInit() ) return err;

    monitoringInit(SLOT(refreshMonitoringOptions()),
                   SLOT(connectChangeEvents()),
                   SLOT(changeAppSelection()),
                   SLOT(changeTypeSelection()),
                   SLOT(changeGroupSelection()),
                   SLOT(changeItemSelection()),
                   SLOT(dataValidityChanged()),
                   SLOT(updateText())
                   );

    this->LH_Text::connect( value_str_obj(), SIGNAL(changed()), this, SLOT(updateText()) );
    this->LH_Text::connect( value_str_obj(), SIGNAL(set()), this, SLOT(updateText()) );

    setup_value_round_ = new LH_Qt_bool(this,"Round",false, LH_FLAG_AUTORENDER);
    setup_value_round_->setHelp( "<p>Round non integer values.</p>");
    setup_value_round_->setOrder(-3);
    this->LH_Text::connect( setup_value_round_, SIGNAL(changed()), this, SLOT(updateText()) );

    setup_append_units_ = new LH_Qt_bool(this, "Append Units", true, 0);
    setup_append_units_->setHelp( "<p>Append the units to the text.</p>");
    setup_append_units_->setOrder(-3);
    connect( setup_append_units_, SIGNAL(changed()), this, SLOT(updateText()) );

    setup_pre_text_ = new LH_Qt_QString( this, "Pre-Text", "", LH_FLAG_AUTORENDER);
    setup_pre_text_->setHelp( "<p>Text to be displayed before the sensor value.</p>");
    setup_pre_text_->setOrder(-3);
    connect( setup_pre_text_, SIGNAL(changed()), this, SLOT(updateText()) );

    setup_post_text_ = new LH_Qt_QString(this, "Post-Text", "", LH_FLAG_AUTORENDER);
    setup_post_text_->setHelp( "<p>Text to be displayed after the sensor value.</p>");
    setup_post_text_->setOrder(-3);
    connect( setup_post_text_, SIGNAL(changed()), this, SLOT(updateText()) );

    (new LH_Qt_QString(this,("image-hr2"), QString("<hr>"), LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_HIDETITLE,lh_type_string_html ))->setOrder(-3);

    setup_text_->setFlag( LH_FLAG_HIDDEN, true );
    setup_text_->setFlag( LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA, true );
    setText(" ");

    return 0;
}

void LH_MonitoringText::updateText()
{
    QString val = value_str();
    QString units;

    bool ok;
    double numericValue = value_num(&ok, &units);
    if(ok)
    {
        if(setup_value_round_->value())
            val = QString::number(numericValue,'f',0);
        else
            val = QString::number(numericValue,'f',6).replace(QRegExp("\\.?0*$"),"");
    }

    units = (!setup_append_units_->value()? "" : units);
    val = QString("%1%2%3%4")
            .arg(setup_pre_text_->value())
            .arg(val)
            .arg(ok? units : "")
            .arg(setup_post_text_->value())
            ;
    if(setText(val))
        this->requestRender();
}
