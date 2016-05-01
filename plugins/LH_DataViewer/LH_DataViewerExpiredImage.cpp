/**
  \file     LH_DataViewerExpiredImage.cpp
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

#include <QtGlobal>
#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QString>
#include <QRegExp>
#include <QHash>

#include "LH_DataViewerExpiredImage.h"
#include "LH_Qt_QStringList.h"

LH_PLUGIN_CLASS(LH_DataViewerExpiredImage)

lh_class *LH_DataViewerExpiredImage::classInfo()
{
    static lh_class classinfo =
    {
        sizeof(lh_class),
        "Dynamic/DataViewer",
        "DataViewerExpiredImage",
        "Data Expired Image",
        -1, -1,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classinfo;
}

LH_DataViewerExpiredImage::LH_DataViewerExpiredImage() : data_(this)
{
    setup_file_ = new LH_Qt_QFileInfo( this, tr("File"), QFileInfo(), LH_FLAG_AUTORENDER );
    setup_file_->setOrder(-1);
    connect( setup_file_, SIGNAL(changed()), this, SLOT(fileChanged()) );
    isExpired = true;
}

LH_DataViewerExpiredImage::~LH_DataViewerExpiredImage()
{
    return;
}

int LH_DataViewerExpiredImage::polling()
{
    if (data_.open())
        setVisible(data_.expired);
    else
        setVisible(false);

    return polling_rate;
}

QImage *LH_DataViewerExpiredImage::render_qimage(int w, int h)
{
    if(QImage *img = initImage(w, h))
    {
        if(!(setup_file_ &&
                setup_file_->value().isFile() &&
                img->load(setup_file_->value().absoluteFilePath())))
            img->fill(qRgba(0, 0, 0, 255));
        return img;
    }
    return 0;
}

void LH_DataViewerExpiredImage::fileChanged()
{
    callback(lh_cb_render,NULL);
}

