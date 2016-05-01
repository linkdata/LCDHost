/**
  \file     LH_PieCPUHistogram.cpp
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
#include "LH_QtCPU.h"

class LH_PieCPUHistogram : public LH_Dial
{
    LH_QtCPU cpu_;

public:
    explicit LH_PieCPUHistogram() : LH_Dial(DIALTYPE_PIE), cpu_(this)
    {
        setMin(0.0);
        setMax(10000.0);
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/CPU",
            "SystemCPUHistogramPie",
            "Core Load (Pie)",
            48,48,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };

        return &classInfo;
    }

    int notify(int n, void *p)
    {
        setMax(10000.0*cpu_.count());
        qreal *loads = new qreal[ cpu_.count() ];
        if( loads )
        {
            if(needleCount() != cpu_.count())
            {
                clearNeedles();
                for( int i=0; i<cpu_.count(); i++ )
                    addNeedle("CPU/Core #"+QString::number(i));
            }

            for( int i=0; i<cpu_.count(); i++ )
                loads[i] = cpu_.coreload(i);
            setVal( loads, cpu_.count() );
            delete[] loads;
        }

        return cpu_.notify(n,p);
    }
};

LH_PLUGIN_CLASS(LH_PieCPUHistogram)
