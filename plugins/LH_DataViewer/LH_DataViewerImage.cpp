/**
  \file     LH_DataViewerImage.cpp
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

#include "LH_DataViewerImage.h"
// #include <windows.h>
#include "LH_Qt_QStringList.h"

LH_PLUGIN_CLASS(LH_DataViewerImage)

lh_class *LH_DataViewerImage::classInfo()
{
    static lh_class classinfo =
    {
        sizeof(lh_class),
        "Dynamic/DataViewer",
        "DataViewerImage",
        "Data Image",
        -1, -1,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classinfo;
}

LH_DataViewerImage::LH_DataViewerImage() : data_(this)
{
    setup_lookup_code_ = new LH_Qt_QString(this, "Lookup Code", "");
    connect( setup_lookup_code_, SIGNAL(changed()), this, SLOT(updateImage()) );

    setup_lookup_code_value_ = new LH_Qt_QString(this, "^LookupCodeValue", "", LH_FLAG_READONLY|LH_FLAG_NOSAVE_DATA|LH_FLAG_NOSAVE_LINK);

    setup_file_ = new LH_Qt_QFileInfo( this, tr("File"), QFileInfo(), LH_FLAG_AUTORENDER );
    setup_file_->setOrder(-1);
    connect( setup_file_, SIGNAL(changed()), this, SLOT(fileChanged()) );

    setup_text_ = new LH_Qt_QString( this, tr("~"), QString(), LH_FLAG_READONLY|LH_FLAG_NOSAVE_DATA|LH_FLAG_NOSAVE_LINK|LH_FLAG_HIDDEN|LH_FLAG_AUTORENDER );

    //setup_text_->setOrder(-1);

    imageDefinitions_ = new QHash<QString, QStringList>();
    columnDefinitions_ = new QHash<QString, int>();

    imageCode_X_ = "";
    imageCode_Y_ = "";
}

LH_DataViewerImage::~LH_DataViewerImage()
{
    return;
}

int LH_DataViewerImage::polling()
{
    updateImage();
    return polling_rate;
}

QImage *LH_DataViewerImage::render_qimage(int w, int h)
{
    QFileInfo fi(setup_file_ ? setup_file_->value() : QFileInfo());
    if(fi.isFile())
    {
        if(QImage *img = initImage(w, h))
        {
            const QString imageName(getImageName());
            QDir imagedir(fi.dir());
            if(imageName.isEmpty() || !img->load(imagedir.filePath(imageName)))
                img->fill(qRgba(0, 0, 0, 255));
            return img;
        }
    }
    return 0;

#if 0
    delete image_;
    if( setup_file_->value().isFile() )
    {
        const QString imageName(getImageName());
        if (imageName=="")
        {
            uchar *data = new uchar[4];
            data[0] = 255;
            data[1] = 0;
            data[2] = 0;
            data[3] = 0;

            image_ = new QImage(data,1,1,QImage::Format_ARGB32);
        }
        else
            image_ = new QImage(folderPath + imageName);
    } else
        image_ = new QImage(w,h,QImage::Format_Invalid);
    return image_;
#endif
}

QString LH_DataViewerImage::getImageName()
{
    QStringList imageItem;
    if (imageCode_Y_=="")
        imageItem = QStringList();
    else if (imageDefinitions_->contains(imageCode_Y_))
        imageItem = imageDefinitions_->value(imageCode_Y_);
    else if (imageDefinitions_->contains(""))
        imageItem = imageDefinitions_->value("");
    else
        imageItem = QStringList();

    int columnIndex = 1;
    if (columnDefinitions_->contains(imageCode_X_))
        columnIndex = columnDefinitions_->value(imageCode_X_);

    QString imageName;
    if (imageItem.count()==0)
        imageName = "";
    else
        imageName = imageItem.at(columnIndex);
    return imageName;
}

void LH_DataViewerImage::fileChanged()
{
    setup_file_->value().refresh();
    if( !setup_file_->value().isFile() )
    {
        setup_text_->setValue(tr("No such file."));
        setup_text_->setFlag(LH_FLAG_HIDDEN,false);
        return;
    }
    else
    {
        setup_text_->setFlag(LH_FLAG_HIDDEN,true);
        QFile file( setup_file_->value().filePath() );

        if( file.open( QIODevice::ReadOnly) )
        {
            QTextStream stream(&file);
            QRegExp rx = QRegExp("(?:\\s*)?(?:;.*)?$");
            QString fileContent = stream.readAll();

            QStringList items = fileContent.split('\r',QString::SkipEmptyParts);
            imageDefinitions_->clear();
            columnDefinitions_->clear();

            bool columnsDefined = false;
            foreach (QString item, items)
            {
                item = item.remove(rx);
                if (item!="")
                {
                    if(!columnsDefined && item.contains(":"))
                    {
                        QStringList colNames = item.trimmed().split(':');
                        //qDebug() << "colNames: [" << colNames.join("][") << "]";
                        for (int i = 0; i<colNames.length(); i++)
                            columnDefinitions_->insert(colNames[i],i+1);
                        columnsDefined = true;
                    } else {
                        if (!columnsDefined) {
                            columnDefinitions_->insert("",1);
                            columnsDefined = true;
                        }
                        QStringList parts = item.split('\t',QString::SkipEmptyParts);
                        if (item.startsWith('\t')) parts.insert(0,"");
                        imageDefinitions_->insert(QString(parts.at(0).trimmed()), parts);
                        //qDebug() << "image[" << imageDefinitions_->count()-1 << "] (" << parts.at(0).trimmed() << ") = " << parts.join(",");
                    }
                }
            }
            updateImage(true);
        } else {
            setup_text_->setValue(tr("Unable to open file."));
            setup_text_->setFlag(LH_FLAG_HIDDEN,false);
            return;
        }
    }
}

void LH_DataViewerImage::updateImage(bool rerender)
{
    if (data_.open() && data_.valid(setup_lookup_code_->value()))
    {
        imageCode_Y_ = data_.populateLookupCode(setup_lookup_code_->value().split(',').at(0));
        imageCode_X_ = (!setup_lookup_code_->value().contains(',')? "" : data_.populateLookupCode(setup_lookup_code_->value().split(',').at(1)));

        rerender = (setup_lookup_code_value_->value() != imageCode_Y_+","+imageCode_X_);
        setup_lookup_code_value_->setValue(imageCode_Y_+","+imageCode_X_);
    }
    if (rerender) callback(lh_cb_render,NULL);
}
