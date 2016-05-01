/**
  \file     LH_MonitoringImage.cpp
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

#include "LH_MonitoringImage.h"

#include <QtGlobal>
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QString>
#include <QRegExp>
#include <QHash>

#include "LH_Qt_QStringList.h"

LH_PLUGIN_CLASS(LH_MonitoringImage)

lh_class *LH_MonitoringImage::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        STRINGIZE(MONITORING_FOLDER),
        STRINGIZE(COMMON_OBJECT_NAME)"Image",
        STRINGIZE(COMMON_OBJECT_NAME)" (Image)",
        48,48,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

LH_MonitoringImage::LH_MonitoringImage() : LH_QImage(), LH_MonitoringObject(this, mdmAll, false, false)
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

const char *LH_MonitoringImage::userInit()
{
    if( const char *err = LH_QImage::userInit() ) return err;

    this->LH_QImage::connect( value_str_obj(), SIGNAL(changed()), this, SLOT(updateValue()) );
    this->LH_QImage::connect( value_str_obj(), SIGNAL(set()), this, SLOT(updateValue()) );

    setup_image_file_->setFlags( LH_FLAG_AUTORENDER | LH_FLAG_READONLY | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA );

    setup_value_ = new LH_Qt_QString( this, "Current Value", "N/A", LH_FLAG_READONLY|LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA );
    setup_value_->setOrder(-1);

    (new LH_Qt_QString(this,("image-hr2"), QString("<hr>"), LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSOURCE | LH_FLAG_NOSINK | LH_FLAG_HIDETITLE,lh_type_string_html ))->setOrder(-1);

    add_cf_target(setup_image_file_);
    add_cf_source(setup_value_);


    return 0;
}

void LH_MonitoringImage::updateValue()
{
    setup_value_->setValue(value_str());
}
