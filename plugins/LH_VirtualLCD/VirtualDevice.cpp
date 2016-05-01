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

#include "VirtualDevice.h"

VirtualDevice::VirtualDevice(LH_QtPlugin *drv) :
    LH_QtDevice(drv)
{
    setup_output_ = new LH_Qt_QImage(this,"Output",QImage(),LH_FLAG_HIDDEN);
}

const char* VirtualDevice::open()
{
    setup_output_->setFlag( LH_FLAG_HIDDEN, false );
    return NULL;
}

const char* VirtualDevice::close()
{
    setup_output_->setValue(QImage());
    setup_output_->setFlag( LH_FLAG_HIDDEN, true );
    return NULL;
}

const char* VirtualDevice::render_qimage(QImage *img)
{
    if( img )
    {
        if( depth() == 1 )
        {
            if( img->format() == QImage::Format_Mono ) setup_output_->setValue( *img );
            else setup_output_->setValue(  img->convertToFormat( QImage::Format_Mono ) );
        }
        else if( depth() == 32 )
        {
            if( img->format() == QImage::Format_ARGB32_Premultiplied ) setup_output_->setValue( *img );
            else setup_output_->setValue( img->convertToFormat( QImage::Format_ARGB32_Premultiplied ) );
        }
    }
    return NULL;
}
