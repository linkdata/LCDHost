/**
  \file     LH_Qt_int.h
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2011, Johan Lindh

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Link Data Stockholm nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
  */

#ifndef LH_QT_INT_H
#define LH_QT_INT_H

#include "LH_QtSetupItem.h"

class LH_Qt_int : public LH_QtSetupItem
{
public:
    LH_Qt_int( LH_QtObject *parent, QString name, int value, int min, int max, int flags = 0 )
        : LH_QtSetupItem( parent, name, lh_type_integer, flags )
    {
        item_.data.i = value;
        item_.param.slider.min = min;
        item_.param.slider.max = max;
    }

    LH_Qt_int( LH_QtObject *parent, QString name, int value, int flags = 0 )
        : LH_QtSetupItem( parent, name, lh_type_integer, flags )
    {
        item_.data.i = value;
        item_.param.slider.min = 0;
        item_.param.slider.max = 99;
    }

    int value() const
    {
        return item_.data.i;
    }

    void setMinimum( int min )
    {
        item_.param.slider.min = min;
        refresh();
    }

    void setMaximum( int max )
    {
        item_.param.slider.max = max;
        refresh();
    }

    void setMinMax( int min, int max )
    {
        item_.param.slider.min = min;
        item_.param.slider.max = max;
        refresh();
    }

    void setValue(int i)
    {
        if( item_.data.i != i )
        {
            item_.data.i = i;
            refresh();
            emit set();
        }
    }

    virtual void setup_change()
    {
        emit change( value() );
        LH_QtSetupItem::setup_change();
    }
};

#endif // LH_QT_INT_H
