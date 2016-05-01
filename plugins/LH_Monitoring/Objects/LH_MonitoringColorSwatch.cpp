/**
  \file     LH_MonitoringColorSwatch.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  \legalese
    This module is based on original work by Johan Lindh.

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

#include "LH_MonitoringColorSwatch.h"

LH_PLUGIN_CLASS(LH_MonitoringColorSwatch)

lh_class *LH_MonitoringColorSwatch::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        STRINGIZE(MONITORING_FOLDER),
        STRINGIZE(COMMON_OBJECT_NAME)"ColorSwatch",
        STRINGIZE(COMMON_OBJECT_NAME)" (ColorSwatch)",
        24,24,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

LH_MonitoringColorSwatch::LH_MonitoringColorSwatch() : LH_ColorSwatch(), LH_MonitoringObject(this, mdmNumbers, true, false)
{
    monitoringInit(SLOT(refreshMonitoringOptions()),
                   SLOT(connectChangeEvents()),
                   SLOT(changeAppSelection()),
                   SLOT(changeTypeSelection()),
                   SLOT(changeGroupSelection()),
                   SLOT(changeItemSelection()),
                   SLOT(dataValidityChanged()),
                   SLOT(updateValue())
                   );
}

const char *LH_MonitoringColorSwatch::userInit()
{
    if( const char *err = LH_ColorSwatch::userInit() ) return err;

    this->LH_ColorSwatch::connect( value_str_obj(), SIGNAL(changed()), this, SLOT(updateValue()) );
    this->LH_ColorSwatch::connect( value_str_obj(), SIGNAL(set()), this, SLOT(updateValue()) );

    //this->LH_ColorSwatch::connect( setup_max_, SIGNAL(changed()), this, SLOT(updateValue()) );

    //(new LH_Qt_QString(this,("image-hr2"), QString("<hr>"), LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_HIDETITLE,lh_type_string_html ))->setOrder(-3);

    return 0;
}

void LH_MonitoringColorSwatch::updateBounds()
{
    bool ok;
    SensorGroup* group = selectedSensorGroup(&ok);

    if(ok && group->limits.minimum.exists)
        setup_lowerbound_value_->setValue(group->limits.minimum.value);
    if(ok && group->limits.maximum.exists)
        setup_upperbound_value_->setValue(group->limits.maximum.value);
}

void LH_MonitoringColorSwatch::updateValue()
{
    setup_current_value_->setValue((qreal)value_num());
}
