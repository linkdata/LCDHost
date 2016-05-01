/**
  \file     LH_QuickAction.h
  @author   Andy Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010,2011 Andy Bridges
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

 **/

#ifndef LH_QUICKACTION_H
#define LH_QUICKACTION_H

#include <QTimer>

#include "LH_QtInstance.h"
#include "LH_Qt_InputState.h"
#include "LH_Qt_QFileInfo.h"
#include "LH_Qt_bool.h"
#include "LH_Qt_int.h"

class LH_QuickAction: public LH_QtInstance
{
    Q_OBJECT
    int counter_;
    QTimer *timer_;
protected:
    LH_Qt_InputState *setup_fire_;
    LH_Qt_bool *setup_enable_timeout_;
    LH_Qt_int *setup_timeout_;
    LH_Qt_QString *setup_countdown_;
    LH_Qt_QFileInfo *setup_layout_;

    void updateCountdown();
public:
    LH_QuickAction();
    const char *userInit(){ hide(); return NULL; }


    static lh_class *classInfo();

public slots:
    void doFire(QString key="",int flags=0,int value=0);
    void changeTimeout();
    void doCountdown();
};

#endif // LH_QUICKACTION_H
