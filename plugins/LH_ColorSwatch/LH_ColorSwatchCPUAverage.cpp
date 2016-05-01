/**
  \file     LH_ColorSwatchCPUAverage.cpp
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
#include "LH_QtCPU.h"

class LH_ColorSwatchCPUAverage : public LH_ColorSwatch
{
    LH_QtCPU cpu_;

public:
    LH_ColorSwatchCPUAverage() : LH_ColorSwatch(0, Qt::green, 100, Qt::red, true), cpu_( this )
    {
        cpu_.smoothingHidden(true);
    }

    static lh_class *classInfo()
    {
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "System/CPU",
            "SystemCPUAverageColorSwatch",
            "Average Load (ColorSwatch)",
            24,24,
            lh_object_calltable_NULL,
            lh_instance_calltable_NULL
        };

        return &classInfo;
    }

    int notify(int n, void *p)
    {
        setValue(cpu_.averageload()/100.0);
        callback(lh_cb_render,NULL);
        return LH_ColorSwatch::notify(n,p) | cpu_.notify(n,p) | LH_NOTE_SECOND;
    }
};

LH_PLUGIN_CLASS(LH_ColorSwatchCPUAverage)

