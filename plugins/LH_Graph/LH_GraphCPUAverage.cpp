/**
  \file     LH_GraphCPUAverage.cpp
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
#include "LH_QtCPU.h"

#include "LH_QtPlugin_Graph.h"

class LH_GraphCPUAverage : public LH_Graph
{
    LH_QtCPU cpu_;
    int valCount;
    qreal valCache;
    qreal lastVal;

    int cpuCount() {
        int cpucount = cpu_.count();
        if (cpucount != linesCount()) {
            valCount = 0;
            valCache = 0;
            lastVal = 0;
            clearLines();
            if (cpucount > 0) {
                if (cpucount > 1) {
                    for (int i = 0; i < cpucount; ++i)
                        addLine("CPU" + QString::number(i));
                } else {
                    addLine("CPU");
                }
            }
        }
        return cpucount;
    }

public:
    LH_GraphCPUAverage(LH_QtObject *parent = 0)
        : LH_Graph(gdmHybrid, cpu_average_, parent)
        , cpu_(this)
        , valCount(0)
        , valCache(0)
        , lastVal(0)
    {
        setMin(0.0);
        setMax(100, BoundGrowthFixed);
        setYUnit("%");
        cpu_.smoothingHidden(true);
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/CPU",
            "SystemCPUAverageGraph",
            "Average Load (Graph)",
            48,48,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };

        return &classInfo;
    }

    int notify(int n, void *p)
    {
        if (dataMode() != gdmExternallyManaged) {
            if (cpuCount() > 0) {
                if(n&LH_NOTE_CPU) {
                    valCache+=cpu_.averageload()/100;
                    valCount+=1;
                }
                if(n&LH_NOTE_SECOND) {
                    if (valCount!=0) {
                        lastVal = valCache/valCount;
                        addValue(lastVal);
                    }
                    valCache = 0;
                    valCount = 0;
                }
            }
        }
        else
            callback(lh_cb_render,NULL);
        return LH_Graph::notify(n,p) | cpu_.notify(n,p) | LH_NOTE_SECOND;
    }

    QImage *render_qimage( int w, int h )
    {
        if(QImage *img = LH_Graph::render_qimage(w, h))
        {
            if (cpuCount() > 0)
                drawSingle();
            return img;
        }
        return 0;
    }
};

LH_PLUGIN_CLASS(LH_GraphCPUAverage)
