/**
  \file     LH_DialTest.cpp
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
#include "LH_Qt_float.h"

class LH_DialTest : public LH_Dial
{
protected:
    LH_Qt_float *setup_min_;
    LH_Qt_float *setup_max_;
    LH_Qt_float *setup_val_;

public:
    LH_DialTest()
    {
        setup_min_ = new LH_Qt_float(this,"Minimum",000,0,1000,LH_FLAG_AUTORENDER);
        setup_max_ = new LH_Qt_float(this,"Maximum",100,0,1000,LH_FLAG_AUTORENDER);
        setup_val_ = new LH_Qt_float(this,"Value",010,0,1000,LH_FLAG_AUTORENDER);

        connect(setup_val_, SIGNAL(changed()), this, SLOT(changeVal()));

        setMin(0.0);
        setMax(100.0);
    }

    static lh_class *classInfo()
    {
#ifndef QT_NO_DEBUG
        static lh_class classInfo =
        {
            sizeof(lh_class),
            "Test",
            "TestDial",
            "Test Dial",
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
        Q_UNUSED(n);
        Q_UNUSED(p);

        setMin(setup_min_->value());
        setMax(setup_max_->value());
        setup_val_->setMinimum(setup_min_->value());
        setup_val_->setMaximum(setup_max_->value());

        setVal(setup_val_->value());

        return LH_NOTE_SECOND;
    }

public slots:
    void changeVal()
    {
        setMin(setup_min_->value());
        setMax(setup_max_->value());
        setup_val_->setMinimum(setup_min_->value());
        setup_val_->setMaximum(setup_max_->value());

        setVal(setup_val_->value());

        callback(lh_cb_notify, NULL);
    }

};

LH_PLUGIN_CLASS(LH_DialTest)
