/**
  \file     LH_BarMemPhysical.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

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

#include "LH_Bar.h"

class LH_BarMemPhysical : public LH_Bar
{
public:
    const char *userInit()
    {
        if( !state()->mem_data.tot_phys )
            qWarning() << "LH_BarMemPhysical: no data available";
        setMin(0.0);
        setMax(1000.0);
        return 0;
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/Memory/Physical",
            "SystemMemoryPhysicalBar",
            "Physical memory used (Bar)",
            48,48,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };

        return &classInfo;
    }

    int notify(int n, void *p)
    {
        Q_UNUSED(p);
        if( !n || n&LH_NOTE_MEM )
            requestRender();
        return LH_NOTE_MEM;
    }

    QImage *render_qimage( int w, int h )
    {
        if(state()->mem_data.tot_phys)
        {
            if(QImage *img = LH_Bar::render_qimage(w,h))
            {
                qreal used_mem = ( state()->mem_data.tot_phys - state()->mem_data.free_phys );
                used_mem *= 1000.0;
                used_mem /= (qreal) state()->mem_data.tot_phys;
                drawSingle( used_mem );
                return img;
            }
        }
        return 0;
    }
};

LH_PLUGIN_CLASS(LH_BarMemPhysical)
