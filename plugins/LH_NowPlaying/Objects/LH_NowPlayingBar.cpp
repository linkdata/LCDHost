/**
  \file     LH_NowPlayingBar.cpp
  @author   Johan Lindh <johan@linkdata.se>
  @author   Andy Bridges <andy@bridgesuk.com>
  Copyright (c) 2010 Johan Lindh, Andy Bridges

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


  **/

#include "LH_NowPlayingBar.h"

LH_PLUGIN_CLASS(LH_NowPlayingBar)

LH_NowPlayingBar::LH_NowPlayingBar()
{
    connect( lh_plugin(), SIGNAL(updated_data()), this, SLOT(refresh_pos()) );
    setMin(0);
    setValue(0,1);

    return;
}

lh_class *LH_NowPlayingBar::classInfo()
{
    static lh_class classInfo =
    {
        sizeof(lh_class),
        "3rdParty/Music",
        "NowPlayingBar",
        "Now Playing (Progress Bar)",
        -1, -1,
        lh_object_calltable_NULL,
        lh_instance_calltable_NULL
    };

    return &classInfo;
}

void LH_NowPlayingBar::refresh_pos()
{
    if (player)
    {
        if( setValue(player->GetPosition(), player->GetDuration()) )  callback(lh_cb_render,NULL);
    } else {
        // No player found
        if( setValue( 0,1 ) )  callback(lh_cb_render,NULL);
    }
}

bool LH_NowPlayingBar::setValue(int val,int max)
{
    setMax(max);
    if(value_ != val)
    {
        value_ = val;
        return true;
    } else
        return false;
}

QImage *LH_NowPlayingBar::render_qimage( int w, int h )
{
    if(QImage *img = LH_Bar::render_qimage(w, h))
    {
        drawSingle( value_ );
        return img;
    }
    return 0;
}

