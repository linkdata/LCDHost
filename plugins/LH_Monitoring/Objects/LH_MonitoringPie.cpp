/**
  \file     LH_MonitoringPie.cpp
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

#include "LH_MonitoringPie.h"

LH_PLUGIN_CLASS(LH_MonitoringPie)

lh_class *LH_MonitoringPie::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        STRINGIZE(MONITORING_FOLDER),
        STRINGIZE(COMMON_OBJECT_NAME)"Pie",
        STRINGIZE(COMMON_OBJECT_NAME)" (Pie)",
        48,48

    };

    return &classInfo;
}

const char *LH_MonitoringPie::userInit()
{
    if( const char *err = LH_Dial::userInit() ) return err;
    ui_ = new LH_MonitoringUI(this, mdmPie, true, false);

    updateBounds();

    connect(ui_, SIGNAL(appChanged()), this, SLOT(configChanged()) );
    connect(ui_, SIGNAL(typeChanged()), this, SLOT(configChanged()) );
    connect(ui_, SIGNAL(groupChanged()), this, SLOT(configChanged()) );
    connect(ui_, SIGNAL(itemChanged()), this, SLOT(configChanged()) );
    connect(ui_, SIGNAL(initialized()), this, SLOT(configChanged()) );

    pollTimer_.start();
    return 0;
}

int LH_MonitoringPie::polling()
{
    if(pollTimer_.elapsed()>=190 && ui_ && ui_->data_)
    {
        pollTimer_.restart();
        float deadVal;
        bool hasDead = (ui_->data_->getDeadValue_Transformed(deadVal));

        int count;
        updateBounds();
        ui_->data_->getCount(count);
        if(needleCount() != count)
            updateNeedles();

        if(!ui_->data_->isGroup())
        {
            float currVal=0;
            bool hasData = ui_->data_->getValue(currVal);
            bool isDead = (hasDead && (deadVal == currVal));
            setNeedleVisibility( hasData && !isDead );
            setVal( currVal );
        } else {
            QVector<qreal> currVals;
            for(int i=0; i<count; i++)
            {
                float currVal=0;
                bool hasData = ui_->data_->getValue(currVal, i);
                bool isDead = (hasDead && (deadVal == currVal));
                setNeedleVisibility( hasData && !isDead, i );
                currVals.append(currVal);
            }
            setVal(currVals);
        }
    }
    int basePoll = LH_Dial::polling();
    return (basePoll==0? 200 : basePoll);
}

void LH_MonitoringPie::refresh()
{
    requestRender();
}

void LH_MonitoringPie::configChanged()  {
    updateBounds();
    updateNeedles();
}

void LH_MonitoringPie::updateBounds()
{
    float max = 1;
    if(ui_ && ui_->data_)
        ui_->data_->getUpperLimit(max);

    setMax(max);
    setMin(0);
    requestRender();
}

void LH_MonitoringPie::updateNeedles()
{
    QStringList names;
    if(ui_ && ui_->data_)
        ui_->data_->getNames(names);
    setNeedles(names);
}
