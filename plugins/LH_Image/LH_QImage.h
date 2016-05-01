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

#ifndef LH_QIMAGE_H
#define LH_QIMAGE_H

#include "LH_Image.h"
#include "LH_QtCFInstance.h"

class LH_IMAGE_EXPORT LH_QImage : public LH_QtCFInstance
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
