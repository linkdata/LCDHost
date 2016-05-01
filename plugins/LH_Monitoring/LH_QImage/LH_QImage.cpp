/**
  \file     LH_QImage.cpp
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

#include "LH_QImage.h"
#include <QDebug>
#include "logo_blob.c"

/*
LH_PLUGIN_CLASS(LH_QImage)

lh_class *LH_QImage::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "Static",
        "StaticImage2",
        "Static Image 2",
        48,48
        
        
    };
    return &classInfo;
}
*/

const char *LH_QImage::userInit()
{
    if( const char *err = LH_QtCFInstance::userInit() ) return err;
    setup_image_file_ = new LH_Qt_QFileInfo( this, ("Image"), QFileInfo(), LH_FLAG_AUTORENDER );
    setup_show_placeholder_ = new LH_Qt_bool( this, "Show placholder image when empty", true, LH_FLAG_AUTORENDER | LH_FLAG_BLANKTITLE);
    return 0;
}

bool LH_QImage::loadPlaceholderImage(QImage *img)
{
    return img ? img->loadFromData(_logo_blob_d.data, _logo_blob_d.len) : false;
}

QImage *LH_QImage::render_qimage(int w, int h)
{
    if(QImage *img = initImage(w, h))
    {
        const QFileInfo fi(setup_image_file_ ? setup_image_file_->value() : QFileInfo());
        if(!(fi.isFile() && img->load(fi.absoluteFilePath())))
            if(setup_show_placeholder_ && setup_show_placeholder_->value())
                loadPlaceholderImage(img);
        return img;
    }
    return 0;
}

