/**
  \file     LH_GraphCPUHistogram.cpp
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
#include "QDebug"
#include "LH_QtCPU.h"

#include "LH_QtPlugin_Graph.h"

#include <QVector>

class LH_GraphCPUHistogram : public LH_Graph
{
    QVector<int> m_valCount;
    QVector<qreal> m_valCache;
    QVector<qreal> m_lastVal;
    LH_QtCPU m_cpu;

public:
    explicit LH_GraphCPUHistogram(LH_QtObject *parent = 0)
        : LH_Graph(gdmHybrid, cpu_histogram_, parent)
        , m_cpu(this)
    {
        setMin(0.0);
        setMax(100.0, BoundGrowthFixed);
        setYUnit("%");
        m_cpu.smoothingHidden(true);
    }

    ~LH_GraphCPUHistogram()
    {
        clear();
    }

    int cpuCount()
    {
        int cpucount = m_cpu.count();
        if (cpucount != linesCount()) {
            clear();
            if (cpucount > 0) {
                m_valCount.resize(cpucount);
                m_valCache.resize(cpucount);
                m_lastVal.resize(cpucount);
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

    void clear()
    {
        clearLines();
        m_valCount.clear();
        m_valCache.clear();
        m_lastVal.clear();
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/CPU",
            "SystemCPUHistogramGraph",
            "Core Load (Graph)",
            48,48,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };

        return &classInfo;
    }

    int notify(int n, void *p)
    {
        if (dataMode() != gdmExternallyManaged)
        {
            if (int n = cpuCount()) {
                for(int i = 0; i < n; ++i)
                {
                    if(n&LH_NOTE_CPU)
                    {
                        m_valCache[i] += m_cpu.coreload(i)/100;
                        m_valCount[i] += 1;
                    }
                    if(n&LH_NOTE_SECOND)
                    {
                        if (m_valCount[i] != 0)
                        {
                            m_lastVal[i] = m_valCache[i]/m_valCount[i];
                            addValue(m_lastVal[i], i);
                        }
                        m_valCache[i] = 0;
                        m_valCount[i] = 0;
                    }
                }
            }
        }
        else
            callback(lh_cb_render,NULL);
        return LH_Graph::notify(n,p) | m_cpu.notify(n,p) | LH_NOTE_SECOND;
    }

    QImage *render_qimage( int w, int h )
    {
        if(QImage *img = LH_Graph::render_qimage(w, h))
        {
            if (cpuCount() > 0)
                drawAll();
            return img;
        }
        return 0;
    }
};

LH_PLUGIN_CLASS(LH_GraphCPUHistogram)
