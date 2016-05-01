
/**
  \file     LH_BarMemVirtual.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

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

#include "LH_Bar.h"
#include "LH_QtNetwork.h"

class LH_BarMemVirtual : public LH_Bar
{
public:
    const char *userInit()
    {
        setMin(0.0);
        setMax(1000.0);
        return 0;
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/Memory/Virtual",
            "SystemMemoryVirtualBar",
            "Virtual memory used (Bar)",
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
        if(state()->mem_data.tot_virt)
        {
            if(QImage *img = LH_Bar::render_qimage(w,h))
            {
                qreal used_mem = ( state()->mem_data.tot_virt - state()->mem_data.free_virt );
                drawSingle( used_mem * 1000.0 / (qreal) (state()->mem_data.tot_virt) );
                return img;
            }
        }
        return 0;
    }
};

LH_PLUGIN_CLASS(LH_BarMemVirtual)
