/**
  \file     LH_DialTime.h
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

#ifndef LH_DIALTIME_H
#define LH_DIALTIME_H

#include "LH_Dial.h"
#include <QTime>

class LH_DialTime : public LH_Dial
{
    Q_OBJECT

protected:
    LH_Qt_bool *setup_manual_adjust_;
    LH_Qt_int *setup_adjust_seconds_;
    LH_Qt_int *setup_adjust_minutes_;
    LH_Qt_int *setup_adjust_hours_;

public:
    LH_DialTime();

    virtual const char *userInit();

    static lh_class *classInfo();

    int notify(int n, void *p);

public slots:
    void changeManualAdjust();
    void changeManualSeconds();
    void changeManualMinutes();
    void changeManualHours();
};

#endif // LH_DIALTIME_H
