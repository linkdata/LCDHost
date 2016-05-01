/**
  \file     LH_GraphMemPhysical.cpp
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

class LH_GraphMemPhysical : public LH_Graph {
public:
    LH_GraphMemPhysical(LH_QtObject *parent = 0)
        : LH_Graph(gdmHybrid, mem_physical_, parent)
    {
        setMin(0);
        setMax(0, BoundGrowthFixed);
        setYUnit("GB");
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/Memory/Physical",
            "SystemMemoryPhysicalGraph",
            "Physical memory used (Graph)",
            48,48,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };
        return &classInfo;
    }

    bool hasMemoryData()
    {
        if (state() && state()->mem_data.tot_phys) {
            const qreal wanted_max = state()->mem_data.tot_phys / GRAPH_MEM_UNIT_BASE;
            if (!qFuzzyCompare(max_val(), wanted_max)) {
                setMax(wanted_max, BoundGrowthFixed);
                addLine(classInfo()->name);
            }
            return true;
        }
        return false;
    }

    int notify(int n, void *p)
    {
        Q_UNUSED(p);

        if (hasMemoryData()) {
            if (dataMode() != gdmExternallyManaged) {
                if(!n || n&LH_NOTE_SECOND) {
                    qreal used_mem = ( state()->mem_data.tot_phys - state()->mem_data.free_phys ) / GRAPH_MEM_UNIT_BASE;
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

LH_PLUGIN_CLASS(LH_GraphMemPhysical)
