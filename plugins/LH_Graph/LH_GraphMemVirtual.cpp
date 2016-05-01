/**
  \file     LH_GraphMemVirtual.cpp
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

#include "LH_Graph.h"

#include "LH_QtPlugin_Graph.h"

class LH_GraphMemVirtual : public LH_Graph
{
public:
    LH_GraphMemVirtual(LH_QtObject *parent = 0)
        : LH_Graph(gdmHybrid, mem_virtual_, parent)
    {
        setMin(0);
        setMax(0, BoundGrowthFixed);
        setYUnit("GB");
    }

    bool hasMemoryData()
    {
        if (state() && state()->mem_data.tot_virt) {
            if (max_val() < 1) {
                setMax(state()->mem_data.tot_virt / GRAPH_MEM_UNIT_BASE, BoundGrowthFixed);
                addLine("Virtual memory");
            }
            return true;
        }
        return false;
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/Memory/Virtual",
            "SystemMemoryVirtualGraph",
            "Virtual memory used (Graph)",
            48,48,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };
        return &classInfo;
    }

    int notify(int n, void *p)
    {
        Q_UNUSED(p);

        if(hasMemoryData())
        {
            if (dataMode() != gdmExternallyManaged)
            {
                if(!n || n&LH_NOTE_SECOND)
                {
                    qreal used_mem = ( state()->mem_data.tot_virt - state()->mem_data.free_virt ) / GRAPH_MEM_UNIT_BASE;
                    addValue(used_mem);
                }
            }
            else
                callback(lh_cb_render,NULL);
        }
        return LH_Graph::notify(n,p) | LH_NOTE_SECOND;
    }

    QImage *render_qimage( int w, int h )
    {
        if(QImage *img = LH_Graph::render_qimage(w, h))
        {
            drawSingle();
            return img;
        }
        return 0;
    }
};

LH_PLUGIN_CLASS(LH_GraphMemVirtual)
