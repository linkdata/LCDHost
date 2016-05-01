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

#ifndef LH_QIMAGE_H
#define LH_QIMAGE_H

#include "LH_QtCFInstance.h"
#include "LH_Qt_QFileInfo.h"
#include "LH_Qt_bool.h"

class LH_QImage : public LH_QtCFInstance
{
    Q_OBJECT

protected:
    LH_Qt_QFileInfo *setup_image_file_;
    LH_Qt_bool *setup_show_placeholder_;

    virtual bool loadPlaceholderImage(QImage *img);

public:
    LH_QImage() : LH_QtCFInstance(), setup_image_file_(0), setup_show_placeholder_(0) {}
    const char *userInit();
    QImage *render_qimage( int w, int h );

    //static lh_class *classInfo();
};

#endif // LH_QIMAGE_H
