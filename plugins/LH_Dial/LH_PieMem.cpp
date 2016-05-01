/**
  \file     LH_PieMem.cpp
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

#include "LH_Dial.h"
#include "LH_QtNetwork.h"

class LH_PieMem : public LH_Dial
{
public:
    LH_PieMem() : LH_Dial(DIALTYPE_PIE, "Hidden")
    {
        setMin(0.0);
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/Memory",
            "SystemMemoryPie",
            "Memory used (Pie)",
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
        {
            qreal totalMem = 0;
            if( state()->mem_data.tot_phys )
                totalMem += state()->mem_data.tot_phys;
            if( state()->mem_data.tot_virt )
                totalMem += state()->mem_data.tot_virt;
            setMax(1000.0);
            qreal *mem = new qreal[ 4 ];
            if( mem )
            {
                if(needleCount() != 4)
                {
                    clearNeedles();
                    addNeedle("Used Physical Memory",QColor::fromRgb(192,0,0));
                    addNeedle("Used Virtual Memory",QColor::fromRgb(0,0,192));
                    addNeedle("Free Virtual Memory",QColor::fromRgb(192,192,255,96));
                    addNeedle("Free Physical Memory",QColor::fromRgb(255,192,192,96));
                }

                mem[0] = ( !state()->mem_data.tot_phys? 0 : (state()->mem_data.tot_phys - state()->mem_data.free_phys) / totalMem * 1000.0);
                mem[1] = ( !state()->mem_data.tot_virt? 0 : (state()->mem_data.tot_virt - state()->mem_data.free_virt) / totalMem * 1000.0);
                mem[2] = ( !state()->mem_data.tot_virt? 0 : (state()->mem_data.free_virt) / totalMem * 1000.0);
                mem[3] = ( !state()->mem_data.tot_phys? 0 : (state()->mem_data.free_phys) / totalMem * 1000.0);
                setVal( mem, 4 );
                delete[] mem;
            }
        }
        return LH_NOTE_MEM;
    }
};

LH_PLUGIN_CLASS(LH_PieMem)

