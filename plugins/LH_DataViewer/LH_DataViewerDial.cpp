/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

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

#include "LH_DataViewerDial.h"

LH_PLUGIN_CLASS(LH_DataViewerDial)

lh_class *LH_DataViewerDial::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Dynamic/DataViewer",
        "DataViewerDial",
        "Data Dial",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

LH_DataViewerDial::LH_DataViewerDial() : data_(this)
{
    setup_lookup_code_ = new LH_Qt_QString( this, "Data Template", "");
    setup_lookup_code_->setOrder(-3);

    setup_item_name_ = new LH_Qt_QString( this, "Item" , "", LH_FLAG_READONLY|LH_FLAG_NOSAVE_DATA);
    setup_item_name_->setOrder(-3);

    setup_value_min_ = new LH_Qt_QString(this,"Min Value/Index","0");
    setup_value_min_->setOrder(-3);

    setup_item_name_min_ = new LH_Qt_QString( this, "Min" , "", LH_FLAG_READONLY|LH_FLAG_NOSAVE_DATA);
    setup_item_name_min_->setOrder(-3);

    setup_value_max_ = new LH_Qt_QString(this,"Max Value/Index","100");
    setup_value_max_->setOrder(-3);

    setup_item_name_max_ = new LH_Qt_QString( this, "Max" , "", LH_FLAG_READONLY|LH_FLAG_NOSAVE_DATA);
    setup_item_name_max_->setOrder(-3);

    connect( setup_lookup_code_, SIGNAL(changed()), this, SLOT(updateDial()) );
    connect( setup_value_min_, SIGNAL(changed()), this, SLOT(updateDial()) );
    connect( setup_value_max_, SIGNAL(changed()), this, SLOT(updateDial()) );

    setMin(0.0);
    setMax(100.0);
}

int LH_DataViewerDial::polling()
{
    int pollingRate = LH_Dial::polling();
    if (pollingRate!=0) return pollingRate;
    updateDial();
    return polling_rate;
};

QImage *LH_DataViewerDial::render_qimage( int w, int h )
{
    if(QImage *img = LH_Dial::render_qimage(w, h))
    {
        drawDial();
        return img;
    }
    return 0;
}

void LH_DataViewerDial::updateDial(bool rerender)
{
    QString currTemplate = QString("%1/%2/%3").arg(setup_lookup_code_->value()).arg(setup_value_min_->value()).arg(setup_value_max_->value());
    if (data_.open() && data_.valid(currTemplate))
    {
        qreal newMin = toReal( data_.populateLookupCode(setup_value_min_->value()) , 0);
        setup_item_name_min_->setValue( data_.populateLookupCode(setup_value_min_->value(), true) );
        rerender = rerender | (newMin!=barMin_);
        barMin_ = newMin;
        setMin( barMin_ );

        qreal newMax = toReal( data_.populateLookupCode(setup_value_max_->value()) , 1);
        setup_item_name_max_->setValue( data_.populateLookupCode(setup_value_max_->value(), true) );
        rerender = rerender | (newMin!=barMax_);
        barMax_ = newMax;
        setMax( barMax_ );

        qreal newValue = toReal( data_.populateLookupCode(setup_lookup_code_->value()), 0 );
        setup_item_name_->setValue( data_.populateLookupCode(setup_lookup_code_->value(), true) );
        rerender = rerender | (newValue!=barValue_);
        setVal(newValue);
    }
    if (rerender) callback(lh_cb_render,NULL);
}

qreal LH_DataViewerDial::toReal(QString str, qreal defaultVal)
{
    bool ok;
    qreal result = str.toDouble(&ok);
    if (ok)
        return result;
    else {
        result = str.toFloat(&ok);
        if (ok)
            return result;
        else
            return defaultVal;
    }
}


