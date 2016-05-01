/**
  \file     LH_MonitoringBar.cpp
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

#include "LH_MonitoringBar.h"

LH_PLUGIN_CLASS(LH_MonitoringBar)

lh_class *LH_MonitoringBar::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        STRINGIZE(MONITORING_FOLDER),
        STRINGIZE(COMMON_OBJECT_NAME)"Bar",
        STRINGIZE(COMMON_OBJECT_NAME)" (Bar)",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

LH_MonitoringBar::LH_MonitoringBar(LH_QtObject *parent)
    : LH_Bar(parent)
    , LH_MonitoringObject(this, mdmAll, true, false)
{
    monitoringInit(SLOT(refreshMonitoringOptions()),
                   SLOT(connectChangeEvents()),
                   SLOT(changeAppSelection()),
                   SLOT(changeTypeSelection()),
                   SLOT(changeGroupSelection()),
                   SLOT(changeItemSelection()),
                   SLOT(dataValidityChanged()),
                   SLOT(updateBar())
                   );
}

const char *LH_MonitoringBar::userInit()
{
    if( const char *err = LH_Bar::userInit() ) return err;

    this->LH_Bar::connect( value_str_obj(), SIGNAL(changed()), this, SLOT(updateBar()) );
    this->LH_Bar::connect( value_str_obj(), SIGNAL(set()), this, SLOT(updateBar()) );

    setup_max_ = new LH_Qt_int(this, "Maximum", 100, 0, 99999);
    setup_max_->setHelp( "<p>The bar's maximum value.</p>");
    setup_max_->setOrder(-3);
    this->LH_Bar::connect( setup_max_, SIGNAL(changed()), this, SLOT(updateBounds()) );

    setup_min_ = new LH_Qt_int(this, "Minimum", 0, 0, 99999);
    setup_min_->setHelp( "<p>The bar's minimum value.</p>");
    setup_min_->setOrder(-3);
    this->LH_Bar::connect( setup_min_, SIGNAL(changed()), this, SLOT(updateBounds()) );

    setup_minmax_hr_ = new LH_Qt_QString(this,("image-hr2"), QString("<hr>"), LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_HIDETITLE,lh_type_string_html );
    setup_minmax_hr_->setOrder(-3);

    updateBounds();

    return 0;
}


QImage *LH_MonitoringBar::render_qimage( int w, int h )
{
    if(QImage *img = LH_Bar::render_qimage(w, h))
    {
        bool ok;
        SensorItem si = this->selectedSensor(&ok);
        if(ok)
        {
            if(!si.group)
            {
                qreal valFlt = (value_str().toFloat(&ok));
                if(ok)
                    drawSingle( valFlt );
            } else {
                QVector<qreal> currVals = getValuesVector(false, 0, ok);
                if(ok)
                    drawList(currVals);
            }
        }
        return img;
    }
    return 0;
}

void LH_MonitoringBar::refresh()
{
    requestRender();
}

void LH_MonitoringBar::updateBounds()
{
    bool ok;
    SensorGroup *group = this->selectedSensorGroup(&ok);

    qreal _max = setup_max_->value();
    qreal _min = setup_min_->value();

    bool minExists = false;
    bool maxExists = false;

    if(ok)
    {
        minExists = group->limits.minimum.exists;
        if(group->limits.minimum.exists)
        {
            _min = group->limits.minimum.value;
            setup_min_->setValue(_min);
        }

        maxExists = group->limits.maximum.exists;
        if(group->limits.maximum.exists)
        {
            _max = group->limits.maximum.value;
            setup_max_->setValue(_max);
        }
    }

    setMax(_max);
    setMin(_min);

    //qDebug() << "Min: " << ok << "; minExists: " << minExists << "; min=" << _min << "; min_val():" << min_val() << "; setup_min_:" << setup_min_->value() << "; actual=set:" << (setup_min_->value() == min_val());
    //qDebug() << "Max: " << ok << "; maxExists: " << maxExists << "; max=" << _max << "; max_val():" << max_val() << "; setup_max_:" << setup_max_->value() << "; actual=set:" << (setup_max_->value() == max_val());

    bool _visible = (!minExists || setup_min_->value() == LH_Bar::min_val()) && (!maxExists || setup_max_->value() == LH_Bar::max_val());
    setup_min_->setVisible(_visible);
    setup_max_->setVisible(_visible);
    setup_minmax_hr_->setVisible(_visible);

    requestRender();
}

void LH_MonitoringBar::updateBar()
{
    requestRender();
}
