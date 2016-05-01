
/**
  \file     LH_BarNetOut.cpp
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

class LH_BarNetOut : public LH_Bar
{
    LH_QtNetwork net_;

public:
    LH_BarNetOut() : LH_Bar(), net_(this)
    {
        setMin(0.0);
        setMax(1000.0);
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/Network/Outbound",
            "SystemNetworkOutboundBar",
            "Outbound Bandwidth Usage (Bar)",
            48,48,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };

        return &classInfo;
    }

    int notify(int n, void *p)
    {
        return net_.notify(n,p);
    }

    QImage *render_qimage( int w, int h )
    {
        if(QImage *img = LH_Bar::render_qimage(w,h))
        {
            drawSingle( net_.outPermille() );
            return img;
        }
        return 0;
    }
};

LH_PLUGIN_CLASS(LH_BarNetOut)
