/**
  \file     LH_PieCPUAverage.cpp
  @author   Johan Lindh <johan@linkdata.se>
  Copyright (c) 2009-2010 Johan Lindh

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

class LH_PieCPUAverage : public LH_Dial
{
    LH_QtCPU cpu_;

public:
    LH_PieCPUAverage() : LH_Dial(DIALTYPE_PIE), cpu_( this )
    {
        setMin(0.0);
        setMax(10000.0);
    }

    static lh_class *classInfo()
    {
        /**
            This class shouldn't be available to the release build as it is much less efficient than the
            usual needle-based dial and, frankly, if this class is available someone will realise they
            can do something clever and cool with it without realising it's absorbing a lot of cycles.
            Basically this class is 95% the same as the CPU Average Dial but nowhere near as efficient.
        **/
#ifndef QT_NO_DEBUG
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/CPU",
            "SystemCPUAveragePie",
            "Average Load (Pie)",
            48,48,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };

        return &classInfo;
#else
        return NULL;
#endif
    }

    int notify(int n, void *p)
    {
        setVal( cpu_.averageload() );
        return cpu_.notify(n,p);
    }

};

LH_PLUGIN_CLASS(LH_PieCPUAverage)

