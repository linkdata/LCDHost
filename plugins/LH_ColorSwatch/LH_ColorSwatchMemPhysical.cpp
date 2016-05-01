/**
  \file     LH_ColorSwatchMemPhysical.cpp
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  \legalese
    This module is based on original work by Johan Lindh.

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

#include "LH_ColorSwatch.h"

class LH_ColorSwatchMemPhysical : public LH_ColorSwatch
{
public:
    LH_ColorSwatchMemPhysical():  LH_ColorSwatch(0, Qt::green, 100, Qt::red, true) {}

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/Memory/Physical",
            "SystemMemoryPhysicalColorSwatch",
            "Physical memory used (ColorSwatch)",
            24,24,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };

        return &classInfo;
    }

    int notify(int n, void *p)
    {
        Q_UNUSED(p);
        if( !n || n&LH_NOTE_MEM )
        {
            if( state()->mem_data.tot_phys )
            {
                qreal used_mem = ( state()->mem_data.tot_phys - state()->mem_data.free_phys );
                setValue( used_mem * 100.0 / (qreal) (state()->mem_data.tot_phys) );
            }
        }
        return LH_NOTE_MEM;
    }
};

LH_PLUGIN_CLASS(LH_ColorSwatchMemPhysical)
