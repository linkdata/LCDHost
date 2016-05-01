/**
  \file     LH_DialTime.cpp
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

#include "LH_DialTime.h"

LH_DialTime::LH_DialTime()
{
    setMin(0.0);
    setMax(60.0);
    isClock = true;
}

const char *LH_DialTime::userInit()
{
    LH_Dial::userInit();

    setup_manual_adjust_ = new LH_Qt_bool(this, "Manual Adjust", false);
    setup_manual_adjust_->setHelp( "<p>Manually tweak the time displayed (e.g. to display time from a different time zone).</p>");
    setup_manual_adjust_->setOrder(-10);

    setup_adjust_seconds_ = new LH_Qt_int(this, "Adjust Seconds",0,-60,60, LH_FLAG_HIDDEN);
    setup_adjust_seconds_->setHelp( "<p>Add or subract seconds from the time.</p>");
    setup_adjust_seconds_->setOrder(-10);

    setup_adjust_minutes_ = new LH_Qt_int(this, "Adjust Minutes",0,-60,60, LH_FLAG_HIDDEN);
    setup_adjust_minutes_->setHelp( "<p>Add or subract minutes from the time.</p>");
    setup_adjust_minutes_->setOrder(-10);

    setup_adjust_hours_ = new LH_Qt_int(this, "Adjust Hours",0,-12,12, LH_FLAG_HIDDEN);
    setup_adjust_hours_->setHelp( "<p>Add or subract hours from the time.</p>");
    setup_adjust_hours_->setOrder(-10);

    connect( setup_manual_adjust_, SIGNAL(changed()), this, SLOT(changeManualAdjust()));
    connect( setup_adjust_seconds_, SIGNAL(changed()), this, SLOT(changeManualSeconds()));
    connect( setup_adjust_minutes_, SIGNAL(changed()), this, SLOT(changeManualMinutes()));
    connect( setup_adjust_hours_, SIGNAL(changed()), this, SLOT(changeManualHours()));


    clearNeedles();
    addNeedle("Hour Hand");
    addNeedle("Minute Hand");
    addNeedle("Second Hand");

    setup_type_->setFlag(LH_FLAG_HIDDEN, true);
    setup_type_->setValue(0);

    ticks.fullCircle.clear();
    ticks.fullCircle.append(tickObject(60, 1, 0.05, 0.90));
    ticks.fullCircle.append(tickObject(12, 2, 0.15, 0.80));

    return 0;
}

lh_class *LH_DialTime::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "System/Date and time",
        "SystemTimeDial",
        "Time (Dial)",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

int LH_DialTime::notify(int n, void *p)
{
    Q_UNUSED(n);
    Q_UNUSED(p);

    QTime now = QTime::currentTime();
    if(setup_manual_adjust_->value())
    {
        now = now.addSecs(setup_adjust_seconds_->value());
        now = now.addSecs(60 * setup_adjust_minutes_->value());
        now = now.addSecs(60*60 * setup_adjust_hours_->value());
    }

    if( now.msec() > 500 ) now = now.addSecs(1);

    qreal *values = new qreal[ 3 ];
    int hour = now.hour() % 12;
    values[0] = hour * 5 + ((qreal)now.minute()) / 12;
    values[1] = now.minute();
    values[2] = now.second();

    setVal( values, 3 );
    delete[] values;

    return LH_NOTE_SECOND;
}

void LH_DialTime::changeManualAdjust()
{
    setup_adjust_seconds_->setFlag(LH_FLAG_HIDDEN, !setup_manual_adjust_->value());
    setup_adjust_minutes_->setFlag(LH_FLAG_HIDDEN, !setup_manual_adjust_->value());
    setup_adjust_hours_->setFlag(LH_FLAG_HIDDEN, !setup_manual_adjust_->value());
}
void LH_DialTime::changeManualSeconds()
{
    if(qAbs(setup_adjust_seconds_->value())==60)
    {
        setup_adjust_minutes_->setValue(setup_adjust_minutes_->value()+setup_adjust_seconds_->value()/60);
        setup_adjust_seconds_->setValue(0);
        changeManualMinutes();
    }
}
void LH_DialTime::changeManualMinutes()
{
    if(qAbs(setup_adjust_minutes_->value())==60)
    {
        setup_adjust_hours_->setValue(setup_adjust_hours_->value()+setup_adjust_minutes_->value()/60);
        setup_adjust_minutes_->setValue(0);
        changeManualHours();
    }
}
void LH_DialTime::changeManualHours()
{
    if(qAbs(setup_adjust_hours_->value())==12)
    {
        setup_adjust_hours_->setValue(0);
    }
}


LH_PLUGIN_CLASS(LH_DialTime)

